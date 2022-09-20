import os
import sys
import unittest

sys.path.append(os.getcwd())

import test20

class MsgHandler(test20.frame_Frame_Handler):

    def __init__(self, msgFunc = None):
        test20.frame_Frame_Handler.__init__(self)
        self.msgFunc = msgFunc

    def handle_message_Msg1(self, msg):
        self.msg1 = True
        if (self.msgFunc is not None):
            self.msgFunc(msg)


    def handle_Message(self, msg):
        sys.exit("shouldn't happen")


class TestProtocol(unittest.TestCase):
    def test_1(self):
        m = test20.message_Msg2()
        m.field_f1().setMeters(0.1)
        self.assertEqual(m.field_f1().getMeters(), 0.1)
        self.assertEqual(m.field_f1().getScaled(), 100.0)
        self.assertEqual(m.field_f1().getValue(), 10000)


if __name__ == '__main__':
    unittest.main()


