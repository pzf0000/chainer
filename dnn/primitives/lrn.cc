/*
 *COPYRIGHT
 *All modification made by Intel Corporation: © 2017 Intel Corporation.
 *Copyright (c) 2015 Preferred Infrastructure, Inc.
 *Copyright (c) 2015 Preferred Networks, Inc.
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy
 *of this software and associated documentation files (the "Software"), to deal
 *in the Software without restriction, including without limitation the rights
 *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions:
 *
 *The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *THE SOFTWARE.
 *
 */


#include <glog/logging.h>
#include <iostream>
#include "common.h"
#include "mkldnn.hpp"
#include "tensor.h"
#include "mem.h"
#include "lrn.h"
#include "utils.h"
#include "lrn_fwd.h"
#include "lrn_bwd.h"
#include "prim_factory.h"
#include "reorder_op.h"

using namespace mkldnn;

extern engine cpu_engine;

template<typename T>
LocalResponseNormalization<T>::LocalResponseNormalization()
{
}

template<typename T>
LocalResponseNormalization<T>::~LocalResponseNormalization()
{
}

template<typename T>
std::vector<Tensor *> LocalResponseNormalization<T>::Forward(
                Tensor *src,
                lrn_param_t *pp)
{
    std::vector<Tensor *> outputs;

    // sanity check
    mkldnn::memory::dims src_dims = {pp->src_d1, pp->src_d2, pp->src_d3, pp->src_d4};
    mkldnn::memory::dims dst_dims = {pp->dst_d1, pp->dst_d2, pp->dst_d3, pp->dst_d4};
    assert(src_dims == src->cxx_dims());

    //sanity check for data type
    //assuem all should have same data type as T
    assert(memory_data_type<T>() == src->cxx_data_type());
    
    // get a conv2d fwd from primitive pool
    LocalResponseNormalizationFwd<T> *lrn_forward = NULL;
    lrn_forward = LocalResponseNormalizationFwdFactory<T>::get(src_dims, dst_dims,
                pp->n, pp->k,
                pp->alpha, pp->beta, 
                lrn_algo_convert(pp->algo_kind));
    
    mkldnn::memory::format src_fmt = src->cxx_format(); // src fmt in tensor

    void *src_tmp = src->data();
    void *src_reorder = NULL;
    
    // check wehther fmt is same
    if (src_fmt == lrn_forward->src_fmt_) {
        LOG(INFO) << "lrn forward fmt matched";
    } else {
        LOG(INFO) << "lrn fwd fmt not match, need to reorder";
        LOG(INFO) << "src_fmt=" << src_fmt <<", lrn_forward->src_fmt_=" << lrn_forward->src_fmt_;
        // FIXME: when to free the reordered memory
        ReorderOp<T>* reorder_src_op = ReorderFactory<T>::get(src_dims, src_fmt, lrn_forward->src_fmt_);
        src_reorder = new avx::byte[src->len()];
        reorder_src_op->execute(src_tmp, src_reorder);
        src_tmp = src_reorder;
    }

    // create tensor based on primitive's dst 
    // assume dst and src have same data type
    // Tensor *ws_tensor = new Tensor(dst_dims, src->cxx_data_type(), lrn_forward->ws_fmt_, cpu_engine);
    Tensor *dst_tensor = new Tensor(dst_dims, src->cxx_data_type(), lrn_forward->dst_fmt_, cpu_engine);
    
    // do forward
    // to return workspace
    LOG(INFO) << "ws_dt_=" << lrn_forward->ws_dt_;
    // workspace must be int tensor
    Tensor *ws_tensor = new Tensor((lrn_forward->ws_dims_), lrn_forward->ws_dt_, lrn_forward->ws_fmt_, cpu_engine);

    lrn_forward->execute(src_tmp, dst_tensor->data(), ws_tensor->data());
    outputs.push_back(dst_tensor);
    outputs.push_back(ws_tensor);


    //FIXME here may cause performance issue
    if (src_reorder != NULL)
        delete static_cast<avx::byte *>(src_reorder);
    LOG(INFO) << "Succ exec lrn forward";
    return outputs;
}

template<typename T>
Tensor *LocalResponseNormalization<T>::Backward(
                Tensor *diff_dst,
                Tensor *ws,
                lrn_param_t *pp)
{
    //sanity check
    mkldnn::memory::dims diff_src_dims = {pp->src_d1, pp->src_d2, pp->src_d3, pp->src_d4};
    mkldnn::memory::dims diff_dst_dims = {pp->dst_d1, pp->dst_d2, pp->dst_d3, pp->dst_d4};
    assert(diff_dst_dims == diff_dst->cxx_dims());

    mkldnn::memory::dims ws_dims;
    mkldnn::memory::data_type ws_dt;
    ws_dims = ws->cxx_dims();
    ws_dt = ws->cxx_data_type();

    // sanity check for data type
    // assuem all x/w/b should have same data type as T
    // FIXME
    assert(memory_data_type<T>() == diff_dst->cxx_data_type());

    // get a conv2d bwd data from primitive pool
    LocalResponseNormalizationBwd<T> *lrn_bwd = NULL;
    lrn_bwd = LocalResponseNormalizationBwdFactory<T>::get( diff_src_dims, diff_dst_dims, ws_dims, ws_dt,
            pp->n, pp->k,
            pp->alpha, pp->beta,
            lrn_algo_convert(pp->algo_kind));

    // FIXME: in this model, every call to conv_forward will create a new tensor, when to free???
    mkldnn::memory::format ws_fmt;
    void* ws_tmp;
    void* ws_reorder = NULL;
    ws_fmt = ws->cxx_format();
    ws_tmp = ws->data();
    
    mkldnn::memory::format diff_dst_fmt = diff_dst->cxx_format();
    void* diff_dst_tmp = diff_dst->data();
    void* diff_dst_reorder = NULL;

    if (ws_fmt != lrn_bwd->ws_fmt_) {
        LOG(INFO) << "ws_fmt=" << ws_fmt << ", lrn_bwd->ws_fmt_="<< lrn_bwd->ws_fmt_;
        ReorderOp<T>* reorder_ws_op = ReorderFactory<T>::get(ws_dims, ws_fmt, lrn_bwd->ws_fmt_);
        ws_reorder = new avx::byte[ws->len()];
        reorder_ws_op->execute(ws_tmp, ws_reorder);
        ws_tmp = ws_reorder;
    } 
    if (diff_dst_fmt != lrn_bwd->diff_dst_fmt_) {
        LOG(INFO) << "diff_dst_fmt=" << diff_dst_fmt <<", lrn_bwd->diff_dst_fmt_=" << lrn_bwd->diff_dst_fmt_;
        ReorderOp<T>* reorder_diff_dst_op = ReorderFactory<T>::get(diff_dst_dims, diff_dst_fmt, lrn_bwd->diff_dst_fmt_);
        diff_dst_reorder = new avx::byte[diff_dst->len()];
        reorder_diff_dst_op->execute(diff_dst_tmp, diff_dst_reorder);
        diff_dst_tmp = diff_dst_reorder;
    }

    // create tensor based on selected primitive
    // assume dst and src have same data type
    Tensor *diff_src_tensor = new Tensor(diff_src_dims, diff_dst->cxx_data_type(), lrn_bwd->diff_src_fmt_, cpu_engine);
    
    lrn_bwd->execute(diff_src_tensor->data(), diff_dst_tmp, ws_tmp);

    // free
    if (ws_reorder != NULL)
        delete static_cast<avx::byte *>(ws_reorder);
    if (diff_dst_reorder != NULL)
        delete static_cast<avx::byte *>(diff_dst_reorder);

    return diff_src_tensor;
}


template class LocalResponseNormalization<float>;


// vim: et ts=4 sw=4 cindent cino^=l0,\:0,N-s
