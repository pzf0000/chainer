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


#ifndef _LINEAR_BWD_DATA_H_
#define _LINEAR_BWD_DATA_H_

#include <mkldnn.hpp>
#include <vector>
#include <memory>
#include "op.h"

template <typename T>
class LinearBwdData : public Op<T>
{
public:
    LinearBwdData(mkldnn::memory::dims diff_src_d,
                  mkldnn::memory::dims w_d,
                  mkldnn::memory::dims diff_dst_d);
    ~LinearBwdData();
    /*
     * Linear backward data primitive setup
     * Params:
     * diff_src_d: input, (n,c,h,w)
     * w_d: diff weight, (out_c, in_c, h, w)
     * diff_dst_d: output, (n, out_c, out_h, out_w)
     */
    void setup(mkldnn::memory::dims diff_src_d,
               mkldnn::memory::dims w_d,
               mkldnn::memory::dims diff_dst_d);
    /*
     * Linear backward weights without bias
     */
    void execute(void* diff_src, void* w, void* diff_dst);
public:
    // expected memory format for this primitive instance
    // forward
    mkldnn::memory::format diff_src_fmt_;
    mkldnn::memory::format weights_fmt_;
    mkldnn::memory::format diff_dst_fmt_;

    //linear primitive
    std::shared_ptr<mkldnn::primitive> linear_bwd_data_;
private:
    //MKLDNN memory
    //backward weights
    std::shared_ptr<mkldnn::memory> diff_src_mem_; // gx
    std::shared_ptr<mkldnn::memory> weights_mem_;//w
    std::shared_ptr<mkldnn::memory> diff_dst_mem_; //gy

    //
    std::shared_ptr<mkldnn::stream> bwd_data_stream_;
    std::vector<mkldnn::primitive> bwd_data_primitives_;
    
    //desc & primitive desc
    //backward weights
    std::shared_ptr<mkldnn::inner_product_backward_data::desc> bwd_data_desc_;
    std::shared_ptr<mkldnn::inner_product_backward_data::primitive_desc> bwd_data_pd_;
    
    //FIXME
    //forward hint, will be removed in the future;
    std::shared_ptr<mkldnn::inner_product_forward::desc> fwd_desc_;
    std::shared_ptr<mkldnn::inner_product_forward::primitive_desc> fwd_pd_;

    //memory desc
    //forward & backward can share the same mem desc
    std::shared_ptr<mkldnn::memory::desc> diff_src_md_; //gx
    std::shared_ptr<mkldnn::memory::desc> weights_md_; // W
    std::shared_ptr<mkldnn::memory::desc> diff_dst_md_; //gy
};
#endif //_LINEAR_BWD_DATA_H


































