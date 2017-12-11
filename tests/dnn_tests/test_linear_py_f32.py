import sys
import unittest
import numpy
import dnn._dnn
from dnn._dnn import linear_param_t, Linear_Py_F32

try:
    import testing
    from testing import condition
except Exception as ex:
    print('*** testing directory is missing: %s' % ex)
    sys.exit(-1)


@testing.parameterize(*testing.product({
    'x_dtype': [numpy.float32],
    'W_dtype': [numpy.float32],
}))
class TestLinearPyF32(unittest.TestCase):

    def setUp(self):
        self.W = numpy.random.uniform(
            -1, 1, (2, 3)).astype(self.W_dtype)
        self.b = numpy.random.uniform(
            -1, 1, 2).astype(self.x_dtype)

        self.x = numpy.random.uniform(-1, 1, (4, 3)).astype(self.x_dtype)
        self.gy = numpy.random.uniform(-1, 1, (4, 2)).astype(self.x_dtype)

        self.check_forward_options = {'atol': 5e-4, 'rtol': 5e-3}
        self.check_backward_options = {'atol': 5e-4, 'rtol': 5e-3}

    def check_forward(self, x, W, b, y_expect):
        lp = linear_param_t()
        lp.with_bias = True if b is not None else False
        lp.with_weights_opt = True

        x = dnn._dnn.mdarray(x)
        W = dnn._dnn.mdarray(W)
        if lp.with_bias:
            b = dnn._dnn.mdarray(b)
            y_act = Linear_Py_F32.Forward(x, W, b, lp)
        else:
            y_act = Linear_Py_F32.Forward(x, W, None, lp)

        y_act = numpy.array(y_act, dtype=self.x_dtype)
        numpy.testing.assert_allclose(
            y_expect, y_act, **self.check_forward_options)

    @condition.retry(3)
    def test_forward_cpu(self):
        self.check_forward(self.x, self.W, self.b,
                           self.x.dot(self.W.T) + self.b)

    @condition.retry(3)
    def test_forward_cpu_nobias(self):
        self.check_forward(self.x, self.W, None, self.x.dot(self.W.T))

    def check_backward_data(self, x, W, gy):
        gx_expect = gy.dot(W).astype(gy.dtype, copy=False)

        lp = linear_param_t()
        lp.with_bias = False
        lp.src_ndims = 2
        W = dnn._dnn.mdarray(W)
        gy = dnn._dnn.mdarray(gy)
        gx_act = Linear_Py_F32.BackwardData(W, gy, lp)
        gx_act = numpy.array(gx_act, dtype=self.W_dtype)
        gx_expect = gy.dot(W).astype(gy.dtype, copy=False)
        numpy.testing.assert_allclose(
            gx_expect, gx_act, **self.check_backward_options)

    @condition.retry(3)
    def test_backward_cpu_data(self):
        self.check_backward_data(self.x, self.W, self.gy)

    def check_backward_weights(self, x, gy):
        gW_expect = gy.T.dot(x).astype(self.W_dtype, copy=False)

        x = dnn._dnn.mdarray(x)
        gy = dnn._dnn.mdarray(gy)
        # create linear parameter
        # for IA specific
        lp = linear_param_t()
        lp.with_bias = False
        lp.src_ndims = 2
        # only calculate gW, no gb
        (gW_act,) = Linear_Py_F32.BackwardWeights(x, gy, lp)
        gW_act = numpy.array(gW_act, dtype=self.W_dtype)

        numpy.testing.assert_allclose(
            gW_expect, gW_act, **self.check_backward_options)

    @condition.retry(3)
    def test_backward_cpu_weights(self):
        self.check_backward_weights(self.x, self.gy)


testing.run_module(__name__, __file__)
