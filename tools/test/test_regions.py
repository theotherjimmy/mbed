"""
mbed SDK
Copyright (c) 2016 ARM Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import unittest
from mock import patch

from intelhex import IntelHex

from tools.utils import ToolException
from tools.notifier.mock import MockNotifier
from tools.config import Region
from tools.regions import merge_region_list, _fill_header


class RegionsTests(unittest.TestCase):
    """
    Test cases for region merging
    """

    def setUp(self):
        """
        Called before each test case

        :return:
        """
        pass

    def tearDown(self):
        """
        Called after each test case

        :return:
        """
        pass

    @patch('tools.regions.intelhex_offset')
    @patch('tools.config')
    def test_merge_region_no_fit(self, mock_config, mock_intelhex_offset):
        """
        Test that merge_region_list call fails when part size overflows region
        size.
        :param mock_config: config object that is mocked.
        :param mock_intelhex_offset: mocked intel_hex_offset call.
        :return:
        """
        max_addr = 87444
        # create a dummy hex file with above max_addr
        mock_intelhex_offset.return_value = IntelHex({0: 2, max_addr: 0})

        # create application and post-application regions and merge.
        region_application = Region(
            "application", 10000, 86000, True, "random.hex"
        )
        region_post_application = Region(
            "postapplication", 100000, 90000, False, None
        )

        notify = MockNotifier()
        region_list = [region_application, region_post_application]
        # path to store the result in, should not get used as we expect
        # exception.
        res = "./"
        mock_config.target.restrict_size = 90000
        toolexception = False

        try:
            merge_region_list(region_list, res, notify, mock_config)
        except ToolException:
            toolexception = True
        except Exception as e:
            print("%s %s" % (e.message, e.args))

        self.assertTrue(toolexception, "Expected ToolException not raised")

    @patch('zlib.crc32')
    def test_evil_crc(self, mock_crc):
        mock_crc.return_value = 0xFFFFFFFFFFFFFFFFF
        region_application = Region(
            "application", 10000, 86000, True, "random.hex"
        )
        region_header = Region(
            "header", 10000, 86000, False, [
                ["evil", "digest", "CRCITT32be", "header"]
            ]
        )
        _fill_header([region_application, region_header], region_header)


if __name__ == '__main__':
    unittest.main()
