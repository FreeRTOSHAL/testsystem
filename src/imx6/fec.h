struct fec_buffer {
	/**
	 * Receive Descriptor Ring x Start Register
	 * 
	 * RDSR1 points to the beginning of circular receive buffer descriptor queue 1 in external
	 * memory. This pointer must be 64-bit aligned (bits 2–0 must be zero); however, for
	 * optimal performance the pointer should be 512-bit aligned, that is, evenly divisible by 64.
	 */
	uint32_t rdsr; /* 0x0 */
	/**
	 * Transmit Buffer Descriptor Ring 1 Start Register
	 * 
	 * TDSR1 provides a pointer to the beginning of the circular transmit buffer descriptor
	 * queue 1 in external memory. This pointer must be 64-bit aligned (bits 2–0 must be zero);
	 * however, for optimal performance the pointer should be 512-bit aligned, that is, evenly
	 * divisible by 64.
	 */
	uint32_t tdsr; /* 0x4 */
	/**
	 * Maximum Receive Buffer Size Register
	 * 
	 * MRBR1 is a user-programmable register that dictates the maximum size of all ring-1
	 * receive buffers. This value should take into consideration that the receive CRC is always
	 * written into the last receive buffer.
	 * 
	 * - To allow one maximum size frame per buffer, MRBR1 must be set to
	 *   RCR[MAX_FL] or larger.
	 * - R_BUF_SIZE is concatentated with the four least-significant bits of this register and
	 *   are used as the maximum receive buffer size.
	 * - To properly align the buffer, MRBR1 must be evenly divisible by 64. To ensure this,
	 *   set the lower two bits of R_BUF_ZIZE to zero. The lower four bits are already set to
	 * zero by the device.
	 * - To minimize bus usage (descriptor fetches), set MRBR1 greater than or equal to 256
	 *   bytes.
	 */
	uint32_t mrbr; /* 0x8 */
};
#define BUFFER0 2
#define BUFFER1 0
#define BUFFER2 1
struct fec_ring {
	/**
	 * Receive Descriptor Active Register
	 *
	 * RDAR1 is a command register, written by the user, to indicate that the receive descriptor
	 * ring 1 has been updated, that is, that the driver produced empty receive buffers with the
	 * empty bit set.
	 */
	uint32_t rdar; /* 0x0 */
	/**
	 * Transmit Descriptor Active Register
	 * 
	 * TDAR1 is a command register that the user writes to indicate that transmit descriptor ring
	 * 1 has been updated, that is, that transmit buffers have been produced by the driver with
	 * the ready bit set in the buffer descriptor.
	 *
	 * The TDAR register is cleared at reset, when ECR[ETHEREN] transitions from set to
	 * cleared, or when ECR[RESET] is set.
	 */
	uint32_t tdar; /* 0x4 */
};
struct fec_core {
	uint32_t reserved0; 
	/**
	 * Interrupt Event Register
	 * 
	 * When an event occurs that sets a bit in EIR, an interrupt occurs if the corresponding bit in
	 * the interrupt mask register (EIMR) is also set. Writing a 1 to an EIR bit clears it; writing
	 * 0 has no effect. This register is cleared upon hardware reset.
	 */
	uint32_t eir; /* 0x4 */
	/**
	 * Interrupt Mask Register
	 * 
	 * EIMR controls which interrupt events are allowed to generate actual interrupts. A
	 * hardware reset clears this register. If the corresponding bits in the EIR and EIMR
	 * registers are set, an interrupt is generated. The interrupt signal remains asserted until a 1
	 * is written to the EIR field (write 1 to clear) or a 0 is written to the EIMR field.
	 */
	uint32_t eimr; /* 0x8 */
	uint32_t reserved1; /* 0xc */
	/**
	 * Receive Descriptor Active Register 
	 *
	 * RDAR is a command register, written by the user, to indicate that the receive descriptor
	 * ring 0 has been updated, that is, that the driver produced empty receive buffers with the
	 * empty bit set.
	 */
	uint32_t rdar; /* 0x10 */
	/**
	 * Transmit Descriptor Active Register
	 *
	 * The TDAR is a command register that the user writes to indicate that the transmit
	 * descriptor ring 0 has been updated, that is, that transmit buffers have been produced by
	 * the driver with the ready bit set in the buffer descriptor.
	 * The TDAR register is cleared at reset, when ECR[ETHEREN] transitions from set to
	 * cleared, or when ECR[RESET] is set.
	 */
	uint32_t tdar; /* 0x14 */
	uint32_t reserved2[3]; /* 0x18 - 0x20 */
	/**
	 * Ethernet Control Register
	 *
	 * ECR is a read/write user register, though hardware may also alter fields in this register. It
	 * controls many of the high level features of the Ethernet MAC, including legacy FEC
	 * support through the EN1588 field.
	 */
	uint32_t ecr; /* 0x24 */
	uint32_t reserved3[6]; /* 0x28 - 0x3C*/
	/**
	 * MII Management Frame Register
	 *
	 * Writing to MMFR triggers a management frame transaction to the PHY device unless
	 * MSCR is programmed to zero.
	 *
	 * If MSCR is changed from zero to non-zero during a write to MMFR, an MII frame is
	 * generated with the data previously written to the MMFR. This allows MMFR and MSCR
	 * to be programmed in either order if MSCR is currently zero.
	 *
	 * If the MMFR register is written while frame generation is in progress, the frame contents
	 * are altered. Software must use the EIR[MII] interrupt indication to avoid writing to the
	 * MMFR register while frame generation is in progress.
	 */
	uint32_t mmfr; /* 0x40 */
	/**
	 * MII Speed Control Register
	 *
	 * MSCR provides control of the MII clock (MDC pin) frequency and allows a preamble
	 * drop on the MII management frame.
	 *
	 * The MII_SPEED field must be programmed with a value to provide an MDC frequency
	 * of less than or equal to 2.5 MHz to be compliant with the IEEE 802.3 MII specification.
	 * The MII_SPEED must be set to a non-zero value to source a read or write management
	 * frame. After the management frame is complete, the MSCR register may optionally be
	 * cleared to turn off MDC. The MDC signal generated has a 50% duty cycle except when
	 * MII_SPEED changes during operation. This change takes effect following a rising or
	 * falling edge of MDC.
	 *
	 * For example, if the internal module clock (i.e., IPS bus clock) is 25 MHz, programming
	 * MII_SPEED to 0x4 results in an MDC as given in the following equation:
	 * MII clock frequency = 25 MHz / ((4 + 1) x 2) = 2.5 MHz
	 *
	 * The following table shows the optimum values for MII_SPEED as a function of IPS bus
	 * clock frequency.
	 */
	uint32_t mscr; /* 0x44 */
	uint32_t reserved4[7]; /* 0x48 - 0x60 */
	/**
	 * MIB Control Register
	 *
	 * MIBC is a read/write register controlling and observing the state of the MIB block.
	 * Access this register to disable the MIB block operation or clear the MIB counters. The
	 * MIB_DIS field resets to 1.MIBC is a read/write register controlling and observing the state of the MIB block.
	 * Access this register to disable the MIB block operation or clear the MIB counters. The
	 * MIB_DIS field resets to 1.
	 */
	uint32_t mibc; /* 0x64 */
	uint32_t reserved5[7]; /* 0x68 - 0x80 */
	/**
	 * Receive Control Register
	 */
	uint32_t rcr; /* 0x84 */
	uint32_t reserved5[15]; /* 0x88 - 0xC0 */
	/**
	 * Transmit Control Register
	 *
	 * TCR is read/write and configures the transmit block. This register is cleared at system
	 * reset. FDEN can only be modified when ECR[ETHEREN] is cleared.
	 */
	uint32_t tcr; /* 0xC4 */
	uint32_t reserved6[7]; /* 0xC8 - 0xE0 */
	/**
	 * Physical Address Lower Register
	 *
	 * ALR contains the lower 32 bits (bytes 0, 1, 2, 3) of the 48-bit address used in the
	 * address recognition process to compare with the destination address (DA) field of receive
	 * frames with an individual DA. In addition, this register is used in bytes 0 through 3 of the
	 * six-byte source address field when transmitting PAUSE frames.
	 */
	uint32_t palr; /* 0xE4 */
	/**
	 * Physical Address Upper Register
	 *
	 * PAUR contains the upper 16 bits (bytes 4 and 5) of the 48-bit address used in the address
	 * recognition process to compare with the destination address (DA) field of receive frames
	 * with an individual DA. In addition, this register is used in bytes 4 and 5 of the six-byte
	 * source address field when transmitting PAUSE frames. Bits 15:0 of PAUR contain a
	 * constant type field (0x8808) for transmission of PAUSE frames.
	 */
	uint32_t paur; /* 0xE8 */
	/**
	 * Opcode/Pause Duration Register
	 *
	 * OPD is read/write accessible. This register contains the 16-bit opcode and 16-bit pause
	 * duration fields used in transmission of a PAUSE frame. The opcode field is a constant
	 * value, 0x0001. When another node detects a PAUSE frame, that node pauses
	 * transmission for the duration specified in the pause duration field. The lower 16 bits of
	 * this register are not reset and you must initialize it.
	 */
	uint32_t opd; /* 0xEC */
	/**
	 * Transmit Interrupt Coalescing Register
	 */
	uint32_t txic[3]; /* 0xF0 - 0xF8 */
	uint32_t reserved7; /* 0xFC */
	/**
	 * Receive Interrupt Coalescing Register
	 */
	uint32_t rxic[3]; /* 0x100 - 0x108 */
	uint32_t reserved7[3]; /* 0x10C - 0x114 */
	/**
	 * Descriptor Individual Upper Address Register
	 *
	 * IAUR contains the upper 32 bits of the 64-bit individual address hash table. The address
	 * recognition process uses this table to check for a possible match with the destination
	 * address (DA) field of receive frames with an individual DA. This register is not reset and
	 * you must initialize it.
	 */
	uint32_t iaur; /* 0x118 */
	/**
	 * Descriptor Individual Lower Address Register
	 *
	 * IALR contains the lower 32 bits of the 64-bit individual address hash table. The address
	 * recognition process uses this table to check for a possible match with the DA field of
	 * receive frames with an individual DA. This register is not reset and you must initialize it.
	 */
	uint32_t ialr; /* 0x11C */
	/**
	 * Descriptor Group Upper Address Register
	 *
	 * GAUR contains the upper 32 bits of the 64-bit hash table used in the address recognition
	 * process for receive frames with a multicast address. You must initialize this register.
	 */
	uint32_t gaur; /* 0x120 */
	/**
	 * Descriptor Group Lower Address Register
	 *
	 * GALR contains the lower 32 bits of the 64-bit hash table used in the address recognition
	 * process for receive frames with a multicast address. You must initialize this register.
	 */
	uint32_t galr; /* 0x124 */
	uint32_t reserved8[7]; /* 0x128 - 0x140 */
	/**
	 * Transmit FIFO Watermark Register
	 * 
	 * If TFWR[STRFWD] is cleared, TFWR[TFWR] controls the amount of data required in
	 * the transmit FIFO before transmission of a frame can begin. This allows you to minimize
	 * transmit latency (TFWR = 00 or 01) or allow for larger bus access latency (TFWR = 11)
	 * due to contention for the system bus. Setting the watermark to a high value minimizes the
	 * risk of transmit FIFO underrun due to contention for the system bus. The byte counts
	 * associated with the TFWR field may need to be modified to match a given system
	 * requirement, for example, worst-case bus access latency by the transmit data uDMA
	 * channel.
	 *
	 * When the FIFO level reaches the value the TFWR field and when the STR_FWD is set to
	 * '0', the MAC transmit control logic starts frame transmission even before the end-of-
	 * frame is available in the FIFO (cut-through operation).
	 * If a complete frame has a size smaller than the threshold programmed with TFWR, the
	 * MAC also transmits the Frame to the line.
	 *
	 * To enable store and forward on the Transmit path, set STR_FWD to '1'. In this case, the
	 * MAC starts to transmit data only when a complete frame is stored in the Transmit FIFO.
	 */
	uint32_t tfwr; /* 0x144 */
	uint32_t reserved8[6]; /* 0x148 - 0x15C */
	/**
	 * Ring Buffer 1, 2 and 0
	 */
	struct fec_buffer buffer[3]; /* 0x160 - 0x188 */
	uint32_t reserved12; /* 0x18C */
	/**
	 * Receive FIFO Section Full Threshold
	 */
	uint32_t rsfl; /* 0x190 */
	/**
	 * Receive FIFO Section Empty Threshold
	 */
	uint32_t rsem; /* 0x194 */
	/**
	 * Receive FIFO Almost Empty Threshold
	 */
	uint32_t raem; /* 0x198 */
	/**
	 * Receive FIFO Almost Full Threshold
	 */
	uint32_t rafl; /* 0x19C */
	/**
	 * Transmit FIFO Section Empty Threshold
	 */
	uint32_t tsem; /* 0x1A0 */
	/**
	 * Transmit FIFO Almost Empty Threshold
	 */
	uint32_t taem; /* 0x1A4 */
	/**
	 * Transmit FIFO Almost Full Threshold
	 */
	uint32_t tafl; /* 0x1A8 */
	/**
	 * Transmit Inter-Packet Gap
	 */
	uint32_t tipg; /* 0x1AC */
	/**
	 * Frame Truncation Length
	 */
	uint32_t ftrl; /* 0x1B0 */
	uint32_t reserved9[3]; /* 0x1B4 - 0x1BC */
	/**
	 * Transmit Accelerator Function Configuration
	 * 
	 * TACC controls accelerator actions when sending frames. The register can be changed
	 * before or after each frame, but it must remain unmodified during frame writes into the
	 * transmit FIFO.
	 * 
	 * The TFWR[STRFWD] field must be set to use the checksum feature.
	 */
	uint32_t tacc; /* 0x1C0 */
	/**
	 * Receive Accelerator Function Configuration
	 */
	uint32_t racc; /* 0x1C4 */
	/**
	 * Receive Classification Match Register for Class n
	 * 
	 * This match register allows specifying up to four priorities, which are tested (OR'ed)
	 * simultaneously. The match detection uses the extracted VLAN field according to the
	 * rules for VLAN detection configured through the ECR register. If both match registers,
	 * RCMR1 and RCMR2, report a match at the same time, only the class 1 match is indicated
	 * as the final result.
	 */
	uint32_t rcmr[2]; /* 0x1C8 - 0x1CC */
	uint32_t reserved10[2]; /* 0x1D0 - 0x1D4 */
	/**
	 * DMA Class Based Configuration
	 * 
	 * The DMA class based configuration registers are used to configure the DMA controller
	 * interface to support the additional class 1 (buffer descriptor ring 1) and class 2 (buffer
	 * descriptor ring 2) traffic and define configuration options such as bandwidth allocation as
	 * needed.
	 */
	uint32_t dmacfg[2]; /* 0x1D8 - 0x1DC */
	/**
	 * Ring Config
	 */
	struct fec_ring ring[2] /* 0x1E0 - 0x1EC */
	/**
	 * QOS Scheme
	 * 
	 * This register sets the QOS scheme.
	 */
	uint32_t qos; /* 0x1F0 */
	uint32_t reserved11[3]; /* 0x1F4 - 0x1FC */
};
struct fec_stat_rmon {
	/**
	 * Packet Count Statistic Register
	 */
	uint32_t rmon_packts; /* 0x4 */
	/**
	 * Broadcast Packets Statistic Register
	 */
	uint32_t rmon_bc_pks; /* 0x8 */
	/**
	 * Multicast Packets Statistic Register
	 */
	uint32_t rmon_mc_pks; /* 0xC */
	/**
	 * Packets with CRC/Align Error Statistic Register
	 */
	uint32_t rmon_crc_algin; /* 0x10 */
	/**
	 * Packets Less Than Bytes and Good CRC Statistic Register
	 */
	uint32_t rmon_undersize; /* 0x14 */
	/**
	 * Packets GT MAX_FL bytes and Good CRC Statistic Register
	 */ 
	uint32_t rmon_oversize; /* 0x18 */
	/**
	 * Packets Less Than 64 Bytes and Bad CRC Statistic Register
	 */
	uint32_t rmon_frag; /* 0x1C */
	/**
	 * Packets Greater Than MAX_FL bytes and Bad CRC Statistic Register
	 */
	uint32_t rmon_jab; /* 0x20 */
	/**
	 * Collision Count Statistic Register
	 */
	uint32_t rmon_col; /* 0x24 */
	/**
	 * 64-Byte Packets Statistic Register
	 */
	uint32_t rmon_p64; /* 0x28 */
	/**
	 * 65- to 127-byte Packets Statistic Register
	 */
	uint32_t rmon_p65to127; /* 0x2C */
	/**
	 * 128- to 255-byte Packets Statistic Register
	 */
	uint32_t rmon_p128to255; /* 0x30 */
	/**
	 * 256- to 511-byte Packets Statistic Register
	 */
	uint32_t rmon_p256to511; /* 0x34 */
	/**
	 * 512- to 1023-byte Packets Statistic Register
	 */
	uint32_t rmon_p512to1023; /* 0x38 */
	/**
	 * 1024- to 2047-byte Packets Statistic Register
	 */
	uint32_t rmon_p1024to2047; /* 0x3C */
	/**
	 * Packets Greater Than 2048 Bytes Statistic Register
	 */
	uint32_t rmon_p_gte2048; /* 0x40 */
	/**
	 * Octets Statistic Register
	 */
	uint32_t rmon_octets; /* 0x44 */
};
struct fec_stat_ieee_tx {
	/**
	 * Reserved Statistic Register
	 */
	uint32_t ieee_drop; /* 0x48 */
	/**
	 * Frames Transmitted OK Statistic Register
	 */
	uint32_t ieee_frame_ok; /* 0x4C */
	/**
	 * Frames Transmitted with Single Collision Statistic Register
	 */
	uint32_t ieee_1col; /* 0x50 */
	/**
	 * Frames Transmitted with Multiple Collisions Statistic Register
	 */
	uint32_t ieee_mcol; /* 0x54 */
	/**
	 * Frames Transmitted after Deferral Delay Statistic Register
	 */
	uint32_t ieee_def; /* 0x58 */
	/**
	 * Frames Transmitted with Late Collision Statistic Register
	 */
	uint32_t ieee_lcol; /* 0x5C */
	/**
	 * Frames Transmitted with Excessive Collisions Statistic Register
	 */
	uint32_t ieee_excol; /* 0x60 */
	/**
	 * Frames Transmitted with Tx FIFO Underrun Statistic Register
	 */
	uint32_t ieee_macerr; /* 0x64 */
	/**
	 * Frames Transmitted with Carrier Sense Error Statistic Register
	 */
	uint32_t ieee_cserr; /* 0x68 */
	/**
	 * Reserved Statistic Register
	 */
	uint32_t ieee_sqe; /* 0x6C */
	/**
	 * Flow Control Pause Frames Transmitted Statistic Register
	 */
	uint32_t ieee_fdxfc; /* 0x70 */
	/**
	 * Octet Count for Frames Transmitted w/o Error Statistic Register
	 */
	uint32_t ieee_octets_ok; /* 0x74 */
};
struct fec_stat_ieee_rx {
	/**
	 * Frames not Counted Correctly Statistic Register
	 */
	uint32_t ieee_drop; /* 0xC8 */
	/**
	 * Frames Received OK Statistic Register
	 */
	uint32_t ieee_frame_ok; /* 0xCC */
	/**
	 * Frames Received with CRC Error Statistic Register
	 */
	uint32_t ieee_crc; /* 0xD0 */
	/**
	 * Frames Received with Alignment Error Statistic Register
	 */
	uint32_t ieee_align; /* 0xD4 */
	/**
	 * Receive FIFO Overflow Count Statistic Register
	 */
	uint32_t ieee_macerr; /* 0xD8 */
	/**
	 * Flow Control Pause Frames Received Statistic Register
	 */
	uint32_t ieee_fdxfc; /* 0xDC */
	/**
	 * Octet Count for Frames Received without Error Statistic Register
	 */
	uint32_t ieee_octets_ok; /* 0xE0 */
}
struct fec_stat {
	uint32_t rmon_t_drop; /* 0x0 */
	struct fec_stat_rmon tx_rmon; /* 0x0 - 0x44 */
	struct fec_stat_ieee_tx tx_ieee; /* 0x48 - 0x74 */
	uint32_t reserved1[3]; /* 0x78 - 0x80 */
	struct fec_stat_rmon rx_rmon; /* 0x84 - 0xC4 */
	struct fec_stat_ieee_rx rx_ieee; /* 0xC8 - 0xE0 */
	uint32_t reserved2[63]; /* 0xE4 - 0x1FC */
};
struct fec_1588 {
	/**
	 * Adjustable Timer Control Register
	 *
	 * ATCR command fields can trigger the corresponding events directly. It is not necessary
	 * to preserve any of the configuration fields when a command field is set in the register,
	 * that is, no read-modify-write is required.
	 */
	uint32_t atcr; /* 0x0 */
	/**
	 * Timer Value Register
	 */
	uint32_t atvr; /* 0x4 */
	/**
	 * Timer Offset Register
	 */
	uint32_t atoff; /* 0x8 */
	/**
	 * Timer Period Register
	 */
	uint32_t atper; /* 0xC */
	/**
	 * Timer Correction Register
	 */
	uint32_t atcor; /* 0x10 */
	/**
	 * Time-Stamping Clock Period Register
	 */
	uint32_t atinc; /* 0x14 */
	/**
	 * Timestamp of Last Transmitted Frame 
	 */
	uint32_t atstmp; /* 0x18 */
	uint32_t reserved[121]; /* 0x18 - 0x1FC */
};
struct fec_capture_timer {
	/**
	 * Timer Control Status Register
	 */
	uint32_t tcsr; /* 0x0 */
	/**
	 * Timer Compare Capture Register
	 */
	uint32_t tccr; /* 0x4 */
};
struct fec_capture {
	uint32_t reserved0; /* 0x0 */
	/**
	 * Timer Global Status Register
	 */
	uint32_t tgsr; /* 0x4 */
	struct fec_capture_timer timer[4]; /* 0x8 - 0x24 */
};
struct fec_reg {
	struct fec_core core;       /* 0x000 - 0x1FC */ 
	struct fec_stat stat;       /* 0x200 - 0x3FC */
	struct fec_1588 ieee1588;   /* 0x400 - 0x5FC */
	struct fec_capture caputre; /* 0x600 - 0x624 */
};
