;*	PAS.INC
;*
;* Include files for Pro Audio Spectrum Sound Device, v1.22
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*

; This file includes necessary pieces from the PAS SDK include files

	.code
PASNORMAL       =0
PASPLUS         =1
PAS16           =2
DEFAULT_BASE    =       00388h          ;; default base I/O address
ALT_BASE_1      =       00384h          ;; first alternate address
ALT_BASE_2      =       0038Ch          ;; second alternate address
ALT_BASE_3      =       00288h          ;; third alternate address
USE_ACTIVE_ADDR =	00000h		;; uses what is currently active

pAUDIOMIXR      =       0078Bh          ;; Parallel interface Audio Mixer Reg
AUDIOMIXR       =       00B88h          ;; Audio Mixer Control Register
AUDIOFILT       =       00B8Ah          ;; Audio Filter Control Register
INTRCTLRST      =       00B89h          ;; Interrupt Control Status Register
INTRCTLR        =       00B8Bh          ;; Interrupt Control Register write
INTRCTLRRB      =       00B8Bh          ;; Interrupt Control Register read back
PCMDATA         =       00F88h          ;; PCM data I/O register (low byte)
PCMDATAH        =       00F89h          ;; PCM data I/O register (high byte)
CROSSCHANNEL    =       00F8Ah          ;; Cross Channel Register
SAMPLERATE      =       01388h          ;; (t0) Sample Rate Timer Register
SAMPLECNT       =       01389h          ;; (t1) Sample Count Register
SPKRTMR         =       0138Ah          ;; (t2) Local Speaker Timer Address
TMRCTLR         =       0138Bh          ;; Local Timer Control Register
MDIRQVECT       =       01788H          ;; MIDI-0 IRQ Vector Register
MDSYSCTLR       =       01789H          ;; MIDI-1 System Control Register
MDSYSSTAT       =       0178AH          ;; MIDI-2 IRQ Status Register
MDIRQCLR        =       0178BH          ;; MIDI-3 IRQ Clear Register
MDGROUP4        =       01B88H          ;; MIDI-4 Group #1 Register (MDGROUP1)
MDGROUP5        =       01B89H          ;; MIDI-5 Group #2 Register (MDGROUP2)
MDGROUP6        =       01B8AH          ;; MIDI-6 Group #3 Register (MDGROUP3)
MDGROUP7        =       01B8BH          ;; MIDI-7 Group #4 Register (MDGROUP4)

;
; mixer select
;
OUTPUTMIXER     =       00h             ;; output mixer H/W select
INPUTMIXER      =       40h             ;; input mixer select
DEFMIXER        =      0ffh              ;; use last mixer selected
MIXERMAX        =       1fh             ;; maximum mixer setting

MVVOLUMEMAX     =       3fh             ;; MVA508 maximum mixer setting
NSVOLUMEMAX     =       28h             ;; National maximum mixer setting

ALIZERMAX    =       0ch             ;; maximum =  alizer setting
ALIZERMID    =       06h             ;; maximum mid setting

;
;       Filter register bits
;
fFIdatabits     =       00011111B       ;; filter select and decode field bits
fFImutebits     =       00100000B       ;; filter mute field bits
fFIpcmbits      =       11000000B       ;; filter sample rate field bits
bFImute         =       00100000B       ;; filter mute bit
bFIsrate        =       01000000B       ;; filter sample rate timer mask
bFIsbuff	=	10000000B	;; filter sample buffer counter mask
FILTERMAX       =       6               ;; six possible settings

;
;       Cross Channel Bit definitions
;
fCCcrossbits    =       00001111B       ;; cross channel bit field
fCCpcmbits      =       11110000B       ;; pcm/dma control bit field
bCCr2r          =       00000001B       ;; CROSSCHANNEL Right to Right
bCCl2r          =       00000010B       ;; CROSSCHANNEL Left  to Right
bCCr2l          =       00000100B       ;; CROSSCHANNEL Right to Right
bCCl2l          =       00001000B       ;; CROSSCHANNEL Left  to Left
bCCdac          =       00010000B       ;; DAC/ADC Control
bCCmono         =       00100000B       ;; PCM Monaural Enable
bCCenapcm       =       01000000B       ;; Enable PCM state machine
bCCdrq          =       10000000B       ;; Enable DRQ bit

;
;       Interrupt Control Register Bits
;
fICintmaskbits  =       00011111B       ;; interrupt mask field bits
fICrevbits      =       11100000B       ;; revision mask field bits
fICidbits       =       11100000B       ;; Board revision ID field bits
bICleftfm	=	00000001B	;; Left FM interrupt enable
bICritfm        =       00000010B       ;; Right FM interrupt enable
bICsamprate     =       00000100B       ;; Sample Rate timer interrupt enable
bICsampbuff     =       00001000B       ;; Sample buffer timer interrupt enable
bICmidi         =       00010000B       ;; MIDI interrupt enable
   fICrevshr    =       5               ;; rotate rev bits to lsb

;
;       Interrupt Status Register Bits
;
fISints         =       00011111B       ;; Interrupt bit field
bISleftfm       =       00000001B       ;; Left FM interrupt active
bISritfm        =       00000010B       ;; Right FM interrupt active
bISsamprate     =       00000100B       ;; Sample Rate timer interrupt active
bISsampbuff     =       00001000B       ;; Sample buffer timer interrupt active
bISmidi         =       00010000B       ;; MIDI interrupt active
bISPCMlr        =       00100000B       ;; PCM left/right active
bISActive       =       01000000B       ;; Hardware is active (not in reset)
bISClip         =       10000000B       ;; Sample Clipping has occured

;
;       cross channel channel #s
;
RIGHT2RIGHT	=	00h		;; right to right
LEFT2RIGHT      =       01h             ;; left  to right
RIGHT2LEFT      =       02h             ;; right to left
LEFT2LEFT       =       03h             ;; left  to left

;
; left/right mixer channel selection
;

;       left channel values

L_FM            =       01h
L_IMIXER        =       02h
L_EXT           =       03h
L_INT           =       04h
L_MIC           =       05h
L_PCM           =       06h
L_SPEAKER       =       07h
L_FREE          =       00h
L_SBDAC         =       00h

;       right channel values

R_FM		=	08h
R_IMIXER        =       09h
R_EXT           =       0Ah
R_INT           =       0Bh
R_MIC           =       0Ch
R_PCM           =       0Dh
R_SPEAKER       =       0Eh
R_FREE          =       0Fh
R_SBDAC         =       0Fh

;       Mixer register bits

fMImixerbits    =       00010111B       ;; mixer control bit fields
fMIspkrbits     =       01000000B       ;; speaker integrator field bits
bMIdata         =       00000001B       ;; data bit
bMIclock        =       00000010B       ;; clock strobe
bMImistrb       =       00000100B       ;; mixer output strobe
bMIna1          =       00001000B       ;; not used
bMIvol          =       00010000B       ;; total volume enabled
bMIna2          =       00100000B       ;; not used
bMIspkrint      =       01000000B       ;; speaker integrator
bMImonofm       =       10000000B       ;; make both FMs mono

bMIfmreset	=	bMIdata 	;; OPL3 FM chip reset
bMIdacreset     =       bMIclock        ;; CODEC reset
bMIsbreset      =       bMImistrb       ;; SB microprocessor reset
bMI508reset     =       bMIvol          ;; MVA508 reset

;       volume control channel #s

VOLMUTE         =       40h             ;; MUTE button
VOLLOUDENH      =       41h             ;; LOUDNESS and ENHANCED STEREO switch
VOLBASS         =       42h             ;; BASS level setting
VOLTREBLE       =       43h             ;; TREBLE level setting
VOLLEFT         =       44h             ;; MASTER LEFT LEVEL settting
VOLRIGHT        =       45h             ;; MASTER RIGHT LEVEL settting
VOLMODE         =       46h             ;; Model Select Left/Stereo/Right

bVOLEbass       =       0001b           ;; enhanced bass bit
bVOLEstereo     =       0010b           ;; enhanced stereo bit

;       output control

pmADDRSELECT    =       80h             ;; Parallel mixer addr select
pmDATASELECT    =       00h             ;; Parallel mixer data select

;	mixer channel programming selection

pmCHANNELLR     =       00h             ;; Left/Right channel select
pmCHANNELL      =       20h             ;; Left  channel only select
pmCHANNELR      =       40h             ;; Right channel only select

;       device select

pmMIXERSELECT   =       10h             ;; Parallel Mixer device select
pmVOLUMESELECT  =       00h             ;; Parallel Volume device select

;       Volume Device selects

pmVOLUMEA       =       01h             ;; Left/Right channel select
pmVOLUMEB       =       02h             ;; Left/Right channel select
pmVOLUMEBASS    =       03h             ;; Left/Right channel select
pmVOLUMETREB    =       04h             ;; Left/Right channel select
pmVOLUMEMODE    =       05h             ;; Left/Right channel select

;       mixer selection

pmOUTPUTMIXER   =       00h             ;; Mixer-A selection
pmINPUTMIXER    =       20h             ;; Mixer-B selection

;       mixer channel swap

pmCHSWAP        =       40h             ;; Mixer channel reroute
					;;
;       int 2F application ID codes

INT2FCODE1      =       0BCH            ;; Bryan's initials

;       int 2F ID (func 0) return register values

INT2FREGBX      =       6D00h           ;; 'm '
INT2FREGCX      =       0076h           ;; ' v'
INT2FREGDX      =       2020h           ;; UPPERCASE XOR MASK

; hardware specific =  ates for the MV101 (digital ASIC)

MASTERADDRP     =       09a01h          ;; Master Address Pointer    (w)
MIDIPRESCALE    =       01788h          ;; MIDI prescale             (r/w)
MIDITIMER       =       01789h          ;; MIDI Timer                (r/w)
MIDIDATA        =       0178ah          ;; MIDI Data                 (r/w)
MIDICONTROL     =       0178bh          ;; MIDI Control              (r/w)
MIDISTATUS      =       01b88h          ;; MIDI Status               (r/w)
MIDIFIFOS	=	01b89h		;; MIDI Fifo Status	     (r/w)
MIDICOMPARE     =       01b8ah          ;; MIDI Compare Time         (r/w)
MIDITEST        =       01b8bh          ;; MIDI Test                 (w)
MASTERCHIPR     =       0ff88h          ;; Master Chip Rev           (r)
SLAVECHIPR      =       0ef88h          ;; Slave Chip Rev            (r)
ENHANCEDSCSI    =       07f89h          ;; Enhanced SCSI detect port
SYSCONFIG1      =       08388h          ;; System Config 1           (r/w)
SYSCONFIG2      =       08389h          ;; System Config 2           (r/w)
SYSCONFIG3      =       0838ah          ;; System Config 3           (r/w)
SYSCONFIG4      =       0838bh          ;; System Config 4           (r/w)
IOCONFIG1       =       0f388h          ;; I/O Config 1              (r/w)
IOCONFIG2       =       0f389h          ;; I/O Config 2              (r/w)
IOCONFIG3       =       0f38ah          ;; I/O Config 3              (r/w)
IOCONFIG4       =       0f38bh          ;; I/O Config 4              (r/w)
COMPATREGE      =       0f788h          ;; Compatible Rgister Enable (r/w)
EMULADDRP       =       0f789h          ;; Emulation Address Pointer (r/w)
WAITSTATE       =       0bf88h          ;; Wait State                (r/w)
PUSHBUTTON      =       0e388h          ;; Push Button (slave)       (???)
AUXINTSTAT      =       0e38ah          ;; Aux Int Status            (???)
AUXINTENA       =       0e38bh          ;; Aux Int Enable            (???)
OVRSMPPRE       =       0bf8ah          ;; Over Sample Prescale      (r/w)
ANALSERD        =       0bf89h          ;; Analog Chip Serial Data   (w)
MASTERMODRD     =       0ff8bh          ;; Master Mode Read          (r)
SLAVEMODRD	=	0ef8bh		;; Slave Mode Read	     (r)
INTWATCHDOG     =       0fb8bh          ;; Interrupt Watch Dog       (???)
MASTERuPDATA    =       0fb88h          ;; Master uP Data            (???)
MASTERuPCMD     =       0fb89h          ;; Master uP Command/Status  (???)
MASTERuPRST     =       0fb8ah          ;; Master uP Restart         (???)
SLAVEuPDATA     =       0eb88h          ;; Slave uP Data             (???)
SLAVEuPCMD      =       0eb88h          ;; Slave uP Command/Status   (???)
SLAVEuPRST      =       0eb88h          ;; Slave uP Restart          (???)
CDTOCOUNTER     =       04388h          ;; CD-ROM timeout counter    (r/w)
CDTOSTAT        =       04389h          ;; CD-ROM timeout status     (r/w)
LEFTVURD        =       02388h          ;; Left VU Read              (r)
RITVURD         =       02389h          ;; Right VU Read             (r)

SBRST           =       00206h          ;; SB Reset                  (w)
SBDATA          =       0020Ah          ;; SB Data Read              (r)
SBCMD           =       0020Ch          ;; SB CMD Write/Status Read  (r/w)
SBSTAT          =       0020Eh          ;; SB Data Status            (r)
MPUDATA         =       00300h          ;; MPU-401 data reg          (r/w)
MPUCMD          =       00301h          ;; MPU-401 command reg       (r/w)

; Sys Config 1

bSC1timena      =       00000001b       ;; shadow enable
bSC1pcmemu	=	00000010b	;; PCM Emulation of PAS1
bSC128mhz       =       00000100b       ;; 28mhz clock divisor
bSC1invcom      =       00001000b       ;; invert COM port interrupt input
bSC1stspea      =       00010000b       ;; stereoize pc speaker
bSC1realsnd     =       00100000b       ;; smart real sound emulatio
bSC1d6          =       01000000b       ;;
bSC1mstrst      =       10000000b       ;; master chip reset

; Sys Config 2

bSC2ovrsmp      =       00000011b       ;; oversampling 0,1,2,4
bSC216bit       =       00000100b       ;; 16 bit audio
bSC212bit       =       00001000b       ;; 12 bit interleaving (d2 must be set too)
bSC2msbinv      =       00010000b       ;; invert MSB from standard method
bSC2slavport    =       01100000b       ;; slave port bits
bSC2vcolock     =       10000000b       ;; VCO locked (Sample Rate Clock Valid)

; Sys Config 3

bSC328mhzfil    =       00000001b       ;; PCM Rate uses 28mhz
bSC31mhzsb      =       00000010b       ;; 1mhz timer for SB sample rate
bSC3vcoinv      =       00000100b       ;; invert VCO output
bSC3bclkinv     =       00001000b       ;; invert BCLK form 16 bit DAC
bSC3lrsync	=	00010000b	;; 0=L/R, 1 = Sync Pulse
bSC3d5          =       00100000b       ;;
bSC3d6          =       01000000b       ;;
bSC3d7          =       10000000b       ;;

; Sys Config 4

bSC4drqahi      =       00000001b       ;; DRQ from drive active high
bSC4dackahi     =       00000010b       ;; DRQ from drive active high
bSC4intahi      =       00000100b       ;; INT from drive active high
bSC4drqvalid    =       00001000b       ;; DRQ line valid from drive
bSC4comena      =       00010000b       ;; enable COM interrupt
bSC4enascsi     =       00100000b       ;; enable SCSI interrupt
bSC4drqptr      =       11000000b       ;; DRQ timing pointer bits

; I/O Config 1

bIC1ps2ena      =       00000001b       ;; Enable Chip (PS2 only)
bIC1comdcd      =       00000110b       ;; COM port decode pointer
bIC1comint      =       00111000b       ;; COM port interrupt pointer
bIC1joyena      =       01000000b       ;; Enable joystick read
bIC1wporena     =       10000000b       ;; Enable warm boot reset

; I/O Config 2

bIC2dmaptr      =       00000111b       ;; DMA channel select

;;bIC28dmaptr   =       00001111b       ;;  8 bit DMA channel select
;;bIC216dmaptr  =       11110000b       ;; 16 bit DMA channel select

; I/O Config 3

bIC3pcmint      =       00001111b       ;; pcm IRQ channel select
bIC3cdint       =       11110000b       ;; cd  IRQ channel select

; Compatibility Register

cpMPUEmulation  =       00000001b       ;; MPU emuation is on bit
cpSBEmulation   =       00000010b       ;; SB emuation is on bit

; Emulation Address Pointer

epSBptr         =       00001111b       ;; bit field for SB emulation
epMPUptr        =       11110000b       ;; bit field for MPU emulation

; Slave Mode Read

bSMRDdrvtyp     =       00000011b       ;; drive interface type
bSMRDfmtyp      =       00000100b       ;; FM chip type
bSMRDdactyp     =       00001000b       ;; 16 bit dac (1) or 8 bit dac (0)
bSMRDimidi      =       00010000b       ;; use internal MIDI
bSMRDswrep      =       10000000b       ;; switch is auto repeating

; Master Mode Read

bMMRDatps2      =       00000001b       ;; AT(1) or PS2(0) bus
bMMRDtmremu     =       00000010b       ;; timer emulation enabled
bMMRDmsmd       =       00000100b       ;; master/slave mode
bMMRDslave      =       00001000b       ;; slave power on or device present
bMMRDattim      =       00010000b       ;; xt/at timing
bMMRDmstrev     =       11100000b       ;; master  rev level

; MIDI Control Register

bMCRenatstmp    =       00000001b       ;; MIDI enable time stamp interrupt
bMCRenacmptm    =       00000010b       ;; MIDI enable compare time interrupt
bMCRenafifoi    =       00000100b       ;; MIDI enable FIFO input interrupt
bMCRenafifoo    =       00001000b       ;; MIDI enable FIFO output interrupt
bMCRenafifooh   =       00010000b       ;; MIDI enable FIFO output half int
bMCRrstfifoi	=	00100000b	;; MIDI reset Input FIFO pointer
bMCRrstfifoo    =       01000000b       ;; MIDI reset Output FIFO pointer
bMCRechoio      =       10000000b       ;; MIDI echo input to output (THRU)

; MIDI Status Register

bMSRtimstamp    =       00000001b       ;; MIDI time stamp interrupt
bMSRcmptime     =       00000010b       ;; MIDI compare time interrupt
bMSRififo       =       00000100b       ;; MIDI input FIFO data avail interrupt
bMSRofifo       =       00001000b       ;; MIDI output FIFO empty interrupt
bMSRofifohalf   =       00010000b       ;; MIDI output FIFO half empty interrupt
bMSRififoovr    =       00100000b       ;; MIDI input FIFO overrun error
bMSRofifoovr    =       01000000b       ;; MIDI output FIFO overrun error
bMSRframeerr    =       10000000b       ;; MIDI frame error

; MIDI FIFO count

bMFCififo       =       00001111b       ;; MIDI input FIFO count
bMFCofifo       =       11110000b       ;; MIDI output FIFO count

; Aux interrupt status/enable

bAUfmrit        =       00000001b       ;; FM right interrupt
bAUpushb	=	00000010b	;; push button active
bAUslavecpu     =       00000100b       ;; slave coprocessor
bAUaux0int      =       00001000b       ;; aux 0 interrupt
bAUaux1int      =       00010000b       ;; aux 1 interrupt
bAUaux2int      =       00100000b       ;; aux 2 interrupt
bAUaux3int      =       01000000b       ;; aux 3 interrupt
bAUmastrcpu     =       10000000b       ;; master coprocessor or emulation active

; Push Buttons on the Front Panel

bPSHmute        =       00000001b       ;; mute on the front panel
bPSHdnarrow     =       00000010b       ;; down arrow on the front panel
bPSHuparrow     =       00000100b       ;; up arrow on the front panel
bPSauxbit1      =       00001000b       ;; unused bit...
bPSauxbit2      =       00010000b       ;; unused bit...
bPSauxbit3      =       00100000b       ;; unused bit...
bPSauxbit4      =       01000000b       ;; unused bit...
bPSauxbit5      =       10000000b       ;; unused bit...




MVState struc
  _sysspkrtmr	 db	0	;    42 System Speaker Timer Address
  _systmrctlr    db     0       ;    43 System Timer Control
  _sysspkrreg    db     0       ;    61 System Speaker Register
  _joystick      db     0       ;   201 Joystick Register
  _lfmaddr       db     0       ;   388 Left  FM Synth Address
  _lfmdata       db     0       ;   389 Left  FM Synth Data
  _rfmaddr       db     0       ;   38A Right FM Synth Address
  _rfmdata       db     0       ;   38B Right FM Synth Data
  _dfmaddr       db     0       ;   788 Dual  FM Synthesizer Address Register
  _dfmdata       db     0       ;   789 Dual  FM Synthesizer Data Register
		 db     0       ;       reserved for future use
  _paudiomixr    db     0       ;   78B Paralllel Audio Mixer Control
  _audiomixr     db     0       ;   B88 Audio Mixer Control
  _intrctlrst    db     0       ;   B89 Interrupt Status
  _audiofilt     db     0       ;   B8A Audio Filter Control
  _intrctlr      db     0       ;   B8B Interrupt Control
  _pcmdata       db     0       ;   F88 PCM Data I/O Register
  _RESRVD2       db     0       ;       reserved
  _crosschannel  db     0       ;   F8A Cross Channel
  _RESRVD3       db     0       ;       reserved
  _samplerate    dw     0       ;  1388 Sample Rate Timer
  _samplecnt     dw     0       ;  1389 Sample Count Register
  _spkrtmr       dw     0       ;  138A Shadow Speaker Timer Count
  _tmrctlr	 db	0	;  138B Shadow Speaker Timer Control
  _mdirqvect     db     0       ;  1788 MIDI IRQ Vector Register
  _mdsysctlr     db     0       ;  1789 MIDI System Control Register
  _mdsysstat     db     0       ;  178A MIDI IRQ Status Register
  _mdirqclr      db     0       ;  178B MIDI IRQ Clear Register
  _mdgroup1      db     0       ;  1B88 MIDI Group #1 Register
  _mdgroup2      db     0       ;  1B89 MIDI Group #2 Register
  _mdgroup3      db     0       ;  1B8A MIDI Group #3 Register
  _mdgroup4      db     0       ;  1B8B MIDI Group #4 Register
ENDS




;;
;; Each product will some/all of of these features
;;

bMVA508         =       0000000000000001b       ; MVA508(1) or National(0)
bMVPS2          =       0000000000000010b       ; PS2 bus stuff
bMVSLAVE        =       0000000000000100b       ; CDPC Slave device is present
bMVSCSI         =       0000000000001000b       ; SCSI interface
bMVENHSCSI      =       0000000000010000b       ; Enhanced SCSI interface
bMVSONY 	=	0000000000100000b	; Sony 535 interface
bMVDAC16        =       0000000001000000b       ; 16 bit DAC
bMVSBEMUL       =       0000000010000000b       ; SB h/w emulation
bMVMPUEMUL      =       0000000100000000b       ; MPU h/w emulation
bMVOPL3         =       0000001000000000b       ; OPL3(1) or 3812(0)
bMV101          =       0000010000000000b       ; MV101 ASIC
bMV101_REV      =       0111100000000000b       ; MV101 Revision
bMV101_MORE     =       1000000000000000b       ; more bits in BX

;;
;; Define the ASIC versions
;;

ASIC_VERSION_B  =       0000000000000010b       ; revision B
ASIC_VERSION_C  =       0000000000000011b       ; revision C
ASIC_VERSION_D  =       0000000000000100b       ; revision D
ASIC_VERSION_E  =       0000000000000101b       ; revision E
ASIC_VERSION_F  =       0000000000000110b       ; revision F

pasSearchHW:
;
; calculate the translation code
;
	mov w[pasport],di
	xor di,DEFAULT_BASE         ; di holds the translation code
;
; grab the version # in the interrupt mask. The top few bits hold the version #
;
	mov dx,INTRCTLR             ; board ID is in MSB 3 bits
	xor dx,di                   ; adjust to other address
	in al,dx
	cmp al,-1                   ; bus float meaning not present?
	je >l1                     ; yes, there is no card here

	mov ah,al                   ; save an original copy
	xor al,fICrevbits           ; the top bits wont change

	out dx,al                   ; send out the inverted bits
	jmp $+2
	jmp $+2
	in al,dx                   ; get it back...

	cmp al,ah                   ; both should match now...
	xchg al,ah                   ; (restore without touching the flags)
	out dx,al

	jnz >l1 		; we have a bad board
	mov ax,1                ; a valid PAS card found
	jmp >l2
l1:     clc                     ; we got here due to a bad board
	ret

l2:     stc
	ret


pasSetVersion:
	mov di,w[pasport]
	xor di,DEFAULT_BASE         ; di holds the translation code
;
; grab the version # in the interrupt mask. The top few bits hold the version #
;
	mov dx,INTRCTLR             ; board ID is in MSB 3 bits
	xor dx,di                   ; adjust to other address
	in al,dx
	cmp al,-1                   ; bus float meaning not present?
	je l1                      ; yes, there is no card here

	mov ah,al                   ; save an original copy
	xor al,fICrevbits	    ; the top bits wont change

	out dx,al                   ; send out the inverted bits
	jmp $+2
	jmp $+2
	jmp $+2
	in al,dx                   ; get it back...

	cmp al,ah                   ; both should match now...
	xchg al,ah                   ; (restore without touching the flags)
	out dx,al

;        jnz @@bad                   ; we have a bad board

	and ax,fICrevbits           ; isolate the ID bits & clear AH
	mov cl,fICrevshr            ; shift the bits into a meaningful
	shr al,cl                   ; position (least signficant bits)
	mov si,ax                   ; save the version #
;
; We do have hardware! Load the product bit definitions
;
	or al,al                   ; is this the first version of h/w?
	jz @@vpas                  ; yes, it's a standard PAS

	; The card is either PAS plus or PAS 16. The only thing that
	; actually matters here is whether the card has a 16-bit DAC or
	; not. If yes, it's a PAS 16, otherwise assume it's PAS plus.

;
; determine the CDROM drive type, FM chip, 8/16 bit DAC, and mixer
;
	mov dx,SLAVEMODRD           ; check for the CDPC
	xor dx,di                   ; modify via the translate code
	in al,dx

	test al,bSMRDdactyp          ; 16 bit DAC?
	jz @@vpasplus              ; no, its an 8 bit DAC - PAS plus

	; neither PAS or PAS plus - assume PAS 16
	mov b[pasCardtype],pas16
	mov b[sdev_bits],128+16
	ret

@@vpas: ; standard PAS card
	mov b[pasCardtype],pasNormal
	mov b[sdev_bits],128+8
	ret

@@vpasplus:
	; PAS plus
	mov b[pasCardtype],pasPlus
	mov b[sdev_bits],128+8
	ret


;/***************************************************************************\
;*
;* Function:    int pasDetect(int *result);
;*
;* Description: Detects Pro Audio Spectrum soundcard
;*
;* Returns:     MIDAS error code.
;*              1 stored to *result if PAS was detected, 0 if not.
;*
;\***************************************************************************/

l1:     stc
	ret
getpas: mov ax,cs
	mov ds,ax
	mov ax,0BC00H               ; make sure MVSOUND.SYS is loaded
	mov bx,'??'                 ; this is our way of knowing if the
	xor cx,cx                   ; hardware is actually present.
	xor dx,dx
	int 2Fh                     ; get the ID pattern
	xor bx,cx                   ; build the result
	xor bx,dx

	cmp bx,'MV'                 ; if not here, exit...
	jne l1
;
; get the MVSOUND.SYS specified DMA and IRQ channel
;
	mov ax,0BC04h               ; get the DMA and IRQ numbers
	int 2Fh
	mov b[pasDMA],bl            ; save the correct DMA & IRQ
	add bl,"0"
	mov b[pasdmatxt],bl
	mov b[pasIRQ],cl
	add cl,"0"
	mov b[pasirqtxt],cl
	; now search for the hardware port address:
	; search the default address
	mov di,DEFAULT_BASE         ; try the first address
	call pasSearchHW
	jc >l2

    ; search the first alternate address

	mov di,ALT_BASE_1           ; try the first alternate
	call pasSearchHW
	jc >l2

    ; search the second alternate address

	mov di,ALT_BASE_2           ; try the second alternate
	call pasSearchHW
	jc >l2

    ; search the third, or user r=  ested alternate address

	mov di,ALT_BASE_3           ; try the third alternate
	call pasSearchHW             ; pass the third A, or user I/O
	jnc l1

l2:	call passetversion
	mov dx,pasname
	mov ah,9
	int 21h
	cmp b[pasCardtype],pasNormal        ; normal PAS or PAS plus?
	je >l5                             ; if yes, 16-bit modes are
	cmp b[pasCardtype],pasPlus          ; not available
	jne >b1
	mov dx,pasplusname
	jmp >l7

	; All output modes available:
b1:     mov dx,pas16name
l7:     mov ah,9
	int 21h
	jmp     >l6

l5:     ; No 16-bit modes available:
l6:     mov dx,pasdetected
	mov ah,9
	int 21h
	mov w[sdev_start],pasinit
	mov w[sdev_stop],pasclose

	db 66h,0b8h             ;Samplingrate korrigieren
	dd 1193181
	db 66h
	xor dx,dx
	db 66h
	xor bx,bx
	mov bx,w[DMSresolution]
	db 66h
	div bx
	db 66h
	mov bx,ax
	db 66h,0b8h
	dd 1193181
	db 66h
	xor dx,dx
	db 66h
	div bx
	mov w[DMSresolution],ax
	clc                     ;PAS found.
	ret

;/***************************************************************************\
;*
;* Function:    int pasInit(ushort mixRate, ushort mode);
;*
;* Description: Initializes Pro Audio Spectrum
;*
;* Input:       mixRate         mixing rate
;*              mode            output mode (see enum sdMode)
;*
;* Returns:     MIDAS error code
;*
;\***************************************************************************/

pasInit:
	; Clear hardware state table:
	mov ax,cs
	mov ds,ax
	mov w[MODtimingcheck],pastimingcheck
	mov     es,ax
	mov     di,offset HardwareShadowTable
	mov     cx,35   ;size mvstate
	xor     al,al
	cld
	rep     stosb

	mov     di,w[pasport]           ; PAS I/O port
	xor     di,DEFAULT_BASE         ; save port XOR default port
	mov     w[_mvTranslateCode],di

;
; setup a pointer to our local hardware state table
;
	mov     bx,offset HardwareShadowTable
	mov     ax,cs
	mov     es,ax
	mov     w[mvhwShadowPointer+0],bx
	mov     w[mvhwShadowPointer+2],es

	mov     b[es:bx+_audiomixr],31h    ; lowest filter setting
	mov     b[es:bx+_crosschannel],09h ; cross channel l-2-l, r-2-r

;
; find the int 2F interface and if found, use it's state table pointer
;
	mov     ax,0BC00H               ; MVSOUND.SYS ID check
	mov     bx,'??'
	xor     cx,cx
	xor	dx,dx
	int     2Fh
	xor     bx,cx
	xor     bx,dx
	cmp     bx,'MV'                 ; is the int 2F interface here?
	jnz     >l7                     ; no, exit home

	mov     ax,0BC02H               ; get the pointer
	int     2fh
	cmp     ax,'MV'                 ; busy or intercepted
	jnz     >l7

	mov     w[mvhwShadowPointer+0],bx
	mov     w[mvhwShadowPointer+2],dx
;
l7:     mov     dx,INTRCTLRST                   ; flush any pending PCM irq
	xor     dx,w[_mvTranslateCode]           ; xlate the board address
	out     dx,al

	; calculate sample rate

	les di,w[offset mvhwShadowPointer]

	db 66h,0b8h	;mov	 eax,1193180
	dd 1193180
	db 66h
	xor     dx,dx
	db 66h
	xor bx,bx
	mov   bx,w[sdev_rate]
	db 66h
	div     bx
	mov     [es:di+_samplerate],ax  ; save output speed
	mov     w[pasSpeed],ax

	test    b[sdev_bits],128        ;Stereo
	jz      >l8
	mov     ax,w[pasSpeed]
	shr     ax,1                    ; multiply output rate with 2 if
	mov     w[pasSpeed],ax           ; stereo
	mov     w[es:di+_samplerate],ax

l8:     db 66h,0b8h     ;mov     eax,1193180
	dd 1193180
	db 66h
	xor     dx,dx
	db 66h
	xor bx,bx
	mov bx,w[pasSpeed]          ; calculate actual output rate
	db 66h
	div bx

	test    b[sdev_bits],128        ;Stereo ?
	jz      >l9                     ; divide with 2 if stereo to get
	db 66h
	shr     ax,1                    ; actual output rate

l9:     mov     w[pasRate],ax

	les di,w[offset mvhwShadowPointer]
	mov     al,00110110b            ; 36h Timer 0 & square wave
	mov     dx,TMRCTLR
	xor     dx,w[_mvTranslateCode]   ; xlate the board address

	cli

	out     dx,al                   ; setup the mode, etc
	mov     b[es:di+_tmrctlr],al

	mov	ax,w[es:di+_samplerate]  ; pre-calculated & saved in
	mov     dx,SAMPLERATE                   ; prior code
	xor     dx,w[_mvTranslateCode]   ; xlate the board address
	out     dx,al                   ; output the timer value

	jmp $+2
	jmp $+2
	jmp $+2

	xchg    ah,al
	out     dx,al
	sti

	mov     dx,CROSSCHANNEL
	xor     dx,w[_mvTranslateCode]

	mov     al,b[es:di+_crosschannel]     ; Stop PAS' DMA transfer
	or      al,bCCdrq
	mov     b[es:di+_crosschannel],al
	out     dx,al

l10:    test b[sdev_bits],128             ; stereo output?
	jz      >l11                    ; if yes, multiply buffer length by
	shl	ax,1			; two - double the number of elements
					; compared to mono
l11:
	les di,w[offset mvhwShadowPointer]
	test b[sdev_bits],16
	je >l13
	mov     dx,SYSCONFIG2
	xor     dx,w[_mvTranslateCode]   ; xlate the board address
	in      al,dx
	and     al,11110011b ; clear the bits
	or      al,100b ; set the appropriate bits
	out     dx,al
l13:    mov     al,bCCmono              ; get the stereo/mono mask bit
	test    b[sdev_bits],128        ;stereo ?
	jz      >l14
	sub     al,al
l14:    or      al,bCCdac               ; get the direction bit mask
	or      al,bCCenapcm            ; enable the PCM state machine
	mov     dx,CROSSCHANNEL
	xor     dx,w[_mvTranslateCode]   ; xlate the board address

	mov     ah,0fh+bCCdrq           ; get a mask to load non PCM bits
	and     ah,b[es:di+_crosschannel]
					; grab all but PCM/DRQ/MONO/DIRECTION
	or      al,ah                   ; merge the two states
	xor     al,bCCenapcm            ; disable the PCM bit
	out     dx,al                   ; send to the hardware
	jmp     $+2
	jmp     $+2
	jmp     $+2
	xor     al,bCCenapcm            ; enable the PCM bit
	out     dx,al                   ; send to the hardware
	mov     b[es:di+_crosschannel],al  ; and save the new state

	xor ax,ax
	mov w[DMAbufferadr],ax
	mov w[DMAbufferlen],ax
	mov w[DMAbufferseg],bufseg

	; Start playing the DMA buffer:
	mov     al,b[pasDMA]
	call StartDMA
;
; Setup the audio filter sample bits
;
	mov     al,b[es:di+_audiofilt]
	or	al,(bFIsrate+bFIsbuff)	; enable the sample count/buff counters
	mov     dx,AUDIOFILT
	xor     dx,w[_mvTranslateCode]   ; xlate the board address
	out     dx,al
	mov     b[es:di+_audiofilt],al

	mov     al,b[es:di+_crosschannel] ; get the state
	mov     dx,CROSSCHANNEL
	xor     dx,w[_mvTranslateCode]   ; xlate the board address
	or      al,bCCdrq               ; set the DRQ bit to control it
	out     dx,al
	mov     b[es:di+_crosschannel],al ; and save the new state

	stc
	ret
l13:    clc
	ret

;/***************************************************************************\
;*
;* Function:    int pasClose()
;*
;* Description: Uninitializes Pro Audio Spectrum
;*
;* Returns:     MIDAS error code
;*
;\***************************************************************************/

pasClose:
;        cmp     [PAS.status],sdOK
;        jne     @@err

	les di,w[offset mvhwShadowPointer]
;
; clear the audio filter sample bits
;
	mov     dx,AUDIOFILT
	xor     dx,w[_mvTranslateCode]     ; xlate the board address
	cli                     ; drop dead...
	mov     al,b[es:di+_audiofilt]    ; get the state
	and     al,not(bFIsrate+bFIsbuff) ; flush the sample timer bits
	mov     b[es:di+_audiofilt],al    ; save the new state
	out     dx,al

	test b[sdev_bits],16
	je      >l1

;
; disable the 16 bit stuff
;
	mov     dx,SYSCONFIG2
	xor     dx,w[_mvTranslateCode]     ; xlate the board address
	in      al,dx
	and     al,low(not bSC216bit+bSC212bit) ; flush the 16 bit stuff
	out     dx,al
;
l1:
;
; clear the appropriate Interrupt Control Register bit
;
	mov     ah,bICsampbuff
	and     ah,low(bICsamprate+bICsampbuff)
	not     ah
	mov     dx,INTRCTLR
	xor     dx,w[_mvTranslateCode]     ; xlate the board address
	in      al,dx
	and     al,ah                   ; kill sample timer interrupts
	out     dx,al
	mov     b[es:di+_intrctlr],al

	mov     al,b[es:di+_crosschannel] ; get the state
	mov     dx,CROSSCHANNEL
	xor     dx,w[_mvTranslateCode]   ; xlate the board address
	and     al,not bCCdrq           ; clear the DRQ bit
	and     al,not bCCenapcm        ; clear the PCM enable bit
	or      al,bCCdac
	out     dx,al

	mov     b[es:di+_crosschannel],al ; and save the new state

	; Stop playing the DMA buffer:
	mov al,b[pasDMA]
	call StopDMA

	stc
	ret

; =======================================
; >>> framecheck for playing routines <<<
; =======================================
pastimingcheck:
	mov al,b[pasdma]
	call GetDMApos
	mov bx,ax
	mov ax,w[modoldmixpos]
	sub ax,bx
	mov bx,w[DMSoutlen]
	cmp ax,bx
	ja >l1
	clc	;no frame to be played
	ret
l1:	stc	;frame needed
	ret

#if !_mdltiny
_text   segment
#endif
pasname:DB"Pro Audio Spectrum $"

pasPlusName:	DB"plus $"
pas16Name:	DB"16 $"
pasdetected:	db"detected on DMA "
pasdmatxt:	db"  and IRQ "
pasirqtxt:	db" .",10,13,"$"
pasPortAddresses:dw 388h,384h,38Ch,288h

	.data
pasSpeed	DW	?		; output rate value
pasRate 	DW	?		; actual output rate
pasMode 	DW	?		; output mode

pasDMAPos	DW	?		; DMA playing position
pasport 	dw ?
pasCardtype	db ?
pasModes	db ?
pasDMA		db ?
pasIRQ		db ?

_mvTranslateCode:dw	 ?	 ; I/O base xor default_base

mvhwShadowPointer:dd	  ?	 ; points to the start of the data table

;
; These variables mirror the hardware state
;
HardwareShadowTable:db 35 dup 0

