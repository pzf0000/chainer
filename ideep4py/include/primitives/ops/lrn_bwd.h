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
 *
 *######################################################################
 *# The CuPy is designed based on NumPy's API.
 *# CuPy's source code and documents contain the original NumPy ones.
 *######################################################################
 *Copyright (c) 2005-2016, NumPy Developers.
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are
 *met:
 *
 *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *    * Neither the name of the NumPy Developers nor the names of any
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *######################################################################
 */


#pragma once
#ifndef _LRN_BWD_H_
#define _LRN_BWD_H_

#include <glog/logging.h>
#include <iostream>
#include <mkldnn.hpp>
#include <vector>
#include "op.h"

template <typename T>
class LocalResponseNormalizationBwd: public Op<T>{
public:
    LocalResponseNormalizationBwd(mkldnn::memory::dims src_d, 
            mkldnn::memory::dims diff_dst_d,
            mkldnn::memory::dims ws_d,
            mkldnn::memory::data_type ws_dt,
            int n, double k, double alpha, double beta,
            mkldnn::algorithm alg_kind); // alg_kind = mkldnn::algorithm::lrn_across_channels

    ~LocalResponseNormalizationBwd();
    
    /*
     * lrn backward primitive setup
     * Params:
     * src_d: src
     * diff_dst_d: diff dst
     */
    void setup(mkldnn::memory::dims src_d, 
               mkldnn::memory::dims diff_dst_d,
               mkldnn::memory::dims ws_d,
               mkldnn::memory::data_type ws_dt,
               int n, double k, double alpha, double beta,
               mkldnn::algorithm alg_kind); // alg_kind = mkldnn::algorithm::lrn_across_channels

    /*
     * lrn backward execute 
     * params:
     * src:
     * diff_src: diff_src
     * diff_dst: diff_dst
     * ws: workspace
     */
    void execute(void *src, void *diff_src, void *diff_dst, void *ws=NULL);

public:
    // expected memory format
    mkldnn::memory::format src_fmt_;
    mkldnn::memory::format diff_src_fmt_;
    mkldnn::memory::format diff_dst_fmt_;
    mkldnn::memory::format ws_fmt_;

    // algo
    mkldnn::algorithm alg_kind_;
private:
    // lrn primitive
    std::shared_ptr<mkldnn::lrn_backward> bwd_;
    std::shared_ptr<mkldnn::stream> bwd_stream_;
    
    // MKL-DNN memory, just dummy data
    std::shared_ptr<mkldnn::memory> src_mem_;
    std::shared_ptr<mkldnn::memory> ws_mem_;
    std::shared_ptr<mkldnn::memory> diff_src_mem_;
    std::shared_ptr<mkldnn::memory> diff_dst_mem_;
    std::shared_ptr<mkldnn::memory::desc> src_md_;
    std::shared_ptr<mkldnn::memory::desc> diff_src_md_;
    std::shared_ptr<mkldnn::memory::desc> diff_dst_md_;

    // fwd hint
    std::shared_ptr<mkldnn::lrn_forward::desc> fwd_desc_;
    std::shared_ptr<mkldnn::lrn_forward::primitive_desc> fwd_pd_;
    
    std::shared_ptr<mkldnn::lrn_backward::desc> bwd_desc_;
    std::shared_ptr<mkldnn::lrn_backward::primitive_desc> bwd_pd_;
    
    std::vector<mkldnn::primitive> bwd_primitives_;
};

#endif // _LRN_BWD_H_


// vim: et ts=4 sw=4 cindent cino^=l0,\:0,N-s
