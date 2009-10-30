#
# Copyright (C) 2009 Ubixum, Inc. 
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
from nitro import DeviceInterface, Terminal, Register, SubReg

di=DeviceInterface(
    name="UXN1212 devices",
    terminal_list=[ 

        Terminal(
            name='FX2',
            comment='Special FX2 functions.  This terminal also gets and sets memory data in the FX2 e600-fdff range.  (Note this does not work with reads, only get/set)', 
            addr=0x06,
            regAddrWidth=16,
            regDataWidth=16,
            register_list = [
                Register(name="version",
                         mode="read",
                         addr=4,
                         comment="FX2 firmware version number",
                         subregs=[ SubReg ( name="minor",
                                        width=8,
                                        comment="FX2 firmware minor version." ),
                                    SubReg ( name="major",
                                        width=8,
                                        comment="FX2 firmware major version." )
                                  ])]
        ),

        Terminal(
            name='FX2_io',
            comment='Access to FX2 Special function registers that control general purpose IO on ports A-E', 
            addr=0x07,
            regAddrWidth=16,
            regDataWidth=16,
            register_list = [
                Register(name="IOA",
                         mode="write",
                         addr=0x80,
                         width=8,
                         comment="Port A IO"),
                Register(name="OEA",
                         mode="write",
                         addr=0xb2,
                         width=8,
                         comment="Output Enable on Port A"),
                Register(name="IOB",
                         mode="write",
                         addr=0x90,
                         width=8,
                         comment="IO on Port B."),
                Register(name="OEB",
                         mode="write",
                         addr=0xb3,
                         width=8,
                         comment="Output Enable on Port B"),
                Register(name="IOC",
                         mode="write",
                         addr=0xA0,
                         width=8,
                         comment="IO on Port C."),
                Register(name="OEC",
                         mode="write",
                         addr=0xb4,
                         width=8,
                         comment="Output Enable on Port C"),
                Register(name="IOD",
                         mode="write",
                         addr=0xB0,
                         width=8,
                         comment="IO on Port D."),
                Register(name="OED",
                         mode="write",
                         addr=0xb5,
                         width=8,
                         comment="Output Enable on Port D"),
                Register(name="IOE",
                         mode="write",
                         addr=0xB1,
                         width=8,
                         comment="IO on Port E."),
                Register(name="OEE",
                         mode="write",
                         addr=0xb6,
                         width=8,
                         comment="Output Enable on Port E")]
             ),

        ########################################################################
        Terminal(
            name='FX2_PROM',
            addr=0x51,
            regAddrWidth=16,
            regDataWidth=8,
            comment='EEPROM for Cypress EZ USB FX2',
            )
     ]
)


