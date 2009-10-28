from nitro import DeviceInterface, Terminal, Register, SubReg

di=DeviceInterface(
    name="fx2 devices",
    terminal_list=[ 

        ########################################################################
        Terminal(
            name='FPGA_bitfile',
            addr=0x02,
            regAddrWidth=16,
            regDataWidth=16,
            comment='Termanial for writing FPGA bit file for programming the FPGA',
            ),
        
        ########################################################################

        Terminal(
            name='SPI_data',
            addr=0x03,
            regAddrWidth=16,
            regDataWidth=16,
            comment="""
                        Use this terminal to read/write data to/from the spi flash prom.
                    """ ),

        Terminal(
            name='SPI_flash',
            comment='SPI flash for programming FPGA from flash prom',
            addr=0x04,
            regAddrWidth=16,
            regDataWidth=16,
            register_list = [
                Register(name='status',
                         type='int',
                         mode='write',
                         addr=0x05,
                         subregs=[
                            SubReg(name="write_in_progress",
                                   width=1,
                                   comment="high while a write transaction is in progress."),
                            SubReg(name="write_enable_latch",
                                   width=1,
                                   comment="high when write instructions can be accepted (set by WREN)." ),
                            SubReg(name="block_protect",
                                   width=3,
                                   comment="Size of area to be software protected against writes." ),
                            SubReg(name="unused",
                                   width=2),
                            SubReg(name="status_write_disable",
                                   width=1,
                                   comment="When W# is low, allows device to be put in write protected mode.")
                         ]
                ),
                Register(name='sector',
                         addr=0x06,
                         width=3,
                         mode="write",
                         comment="SPI has 8 sectors.  Write 0-7 to this to determine which sector reads/writes start in."),
                Register(name="spi_be",
                         addr=0x08,
                         mode="write",
                         type="trigger",
                         comment="Write 1 to this to erase the flash."),
                Register(name="chip_id",
                         mode="read",
                         addr=0x9f,
                         comment="Manufacturer Identification"),


            ]
     ),
     #######################################################################
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
                                  ]),
                Register(name="boot_fpga",
                         type="trigger",
                         mode="write",
                         addr=5,
                         width=1,
                         comment="write 1 to this register to cause the fpga to boot from it's prom.")]

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
                         comment="Port A IO",
                         subregs=[
                            SubReg ( name="init_b",
                                     width=1,
                                     comment="FPGA init_b output." ),
                            SubReg ( name="done",
                                     width=1,
                                     comment="FPGA done output." ),
                            SubReg ( name="cs_flash",
                                    width=1,
                                    init=1,
                                    comment="chip select for spi flash." ),
                            SubReg ( name="prog_b",
                                    width=1,
                                    init=1,
                                    comment="pulse prog_b low to cause fpga to reprogram." ),
                            SubReg ( name="PA4",
                                    width=1,
                                    comment="unused" ),
                            SubReg ( name="CCLK",
                                    width=1,
                                    comment="spi flash clock." ),
                            SubReg ( name="d_flash",
                                    width=1,
                                    comment="spi flash data output." ),
                            SubReg ( name="d0",
                                    width=1,
                                    comment="spi flash data input." ),
                         ]
                         ),
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


