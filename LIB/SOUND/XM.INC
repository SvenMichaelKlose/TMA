; XM header definitions
S3Mheader struc
        xmid            db 17 dup ?     ;ID text: 'Extended module: '
        xmname          db 21 dup ?     ;Module name
        xmtrackername   db 20 dup ?     ;Tracker name
        xmversion       dw ?            ;The current format is version $0103

        xmheadersize    dd ?            ;Header size
        xmsonglength    dw ?            ;Song length (in patten order table)
        xmrestartpos    dw ?            ;Restart position
        xmchannels      dw ?            ;Number of channels (2,4,6,8,10,...,32)
        xmpatterns      dw ?            ;Number of patterns (max 256)
        xminstruments   dw ?            ;Number of instruments (max 128)
        xmfrequency     dw ?            ;Flags: bit 0: 0 = Amiga frequency table
                                        ;              1 = Linear frequency table
        xmtempo         dw ?            ;Default tempo
        xmbpm           dw ?            ;Default BPM
        xmorders        db 256 dup ?    ;Pattern order table
ends

xmpattern struc
        xmpatheadersize dd ?            ;Pattern header length
        xmpacking       db ?            ;Packing type (always 0)
        xmrows          dw ?            ;Number of rows in pattern (1..256)
        xmpatdatasize   dw ?            ;Packed patterndata size
ends

xminstrument struc
        xminstsize      dd ?            ;Instrument size
        xminstname      db 22 dup ?     ;Instrument name
        xminsttype      db ?            ;Instrument type (always 0)
        xminstsamples   dw ?            ;Number of samples in instrument
ends

xminstdesc struc
        xmsamheadersize dd ?            ;Sample header size
        xmsamnotes      db 96 dup ?     ;Sample number for all notes
        xmvolenvelope   db 48 dup ?     ;Points for volume envelope
        xmpanenvelope   db 48 dup ?     ;Points for panning envelope
        xmvolpoints     db ?            ;Number of volume points
        xmpanpoints     db ?            ;Number of panning points
        xmvolsustainpnt db ?            ;Volume sustain point
        xmvolloopstart  db ?            ;Volume loop start point
        xmvolloopend    db ?            ;Volume loop end point
        xmpansustainpnt db ?            ;Panning sustain point
        xmpanloopstart  db ?            ;Panning loop start point
        xmpanloopend    db ?            ;Panning loop end point
        xmvoltype       db ?            ;Volume type: bit 0: On; 1: Sustain; 2: Loop
        xmpantype       db ?            ;Panning type: bit 0: On; 1: Sustain; 2: Loop
        xmvibratotype   db ?            ;Vibrato type
        xmvibratosweep  db ?            ;Vibrato sweep
        xmvibratodepth  db ?            ;Vibrato depth
        xmvibratorate   db ?            ;Vibrato rate
        xmvolumefadeout dw ?            ;Volume fadeout
        xmreserved      dw ?            ;Reserved
ends

struc sampleheader
        xmsamlength     dd ?            ;Sample length
        xmsamloopstart  dd ?            ;Sample loop start
        xmsamlooplength dd ?            ;Sample loop length
        xmsamvolume     db ?            ;Volume
        xmsamfinetune   db ?            ;Finetune (signed byte -16..+15)
        xmsamtype       db ?            ;Type: Bit 0-1: 0 = No loop, 1 = Forward loop,
                                        ;2 = Ping-pong loop;
                                        ;4: 16-bit sampledata
        xmsampanning    db ?            ;Panning (0-255)
        xmrelativenote  db ?            ;Relative note number (signed byte)
        xmreserved2     db ?            ;Reserved
        xmsamname       db 22 dup ?     ;Sample name
ends
