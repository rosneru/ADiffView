	include "dos/dos.i"
	include "exec/memory.i"
	include "graphics/GfxBase.i"
	include "hardware/custom.i"
	include "hardware/cia.i"

	XDEF	_SysBase
	XDEF	_GfxBase
	XDEF	_DOSBase

	;
	; Stating the used LVOs
	; (coming from amiga.lib / small.lib)
	;
	XREF	_AbsExecBase
	XREF	_LVOOpenLibrary
	XREF	_LVOCloseLibrary
	XREF	_LVOAllocVec
	XREF	_LVOFreeVec
	XREF	_LVOLoadView
	XREF	_LVOWaitTOF
	XREF	_LVOForbid
	XREF	_LVOPermit
	XREF	_LVOOwnBlitter
	XREF	_LVODisownBlitter
	XREF	_LVOOpen
	XREF	_LVOClose
	XREF	_LVOIoErr
	XREF	_LVOPrintFault
	XREF	_LVOVPrintf
	XREF	_LVORead
	XREF	_custom
	XREF	_ciaa


*======================================================================
* Definitions
*======================================================================

BGIMGNUMPLANES	EQU	4
BGIMGPLANESIZE	EQU	640/8*256
BGIMGSIZE	EQU	BGIMGPLANESIZE*BGIMGNUMPLANES

DUCKWORDWIDTH	EQU	4
DUCKHEIGHT	EQU	21
DUCKNUMPLANES	EQU	4
DUCKPLANESIZE	EQU	DUCKWORDWIDTH*DUCKHEIGHT*2
DUCKSIZE	EQU	DUCKPLANESIZE*DUCKNUMPLANES
DUCKBLTSIZE	EQU	(DUCKHEIGHT*64)+DUCKWORDWIDTH

*======================================================================
* Initializations
*======================================================================

	movea.l _AbsExecBase,a6
	move.l	a6,_SysBase

	;
	; Open dos.library
	;
	lea	dosname(pc),a1
	moveq	#36,d0		;36 needed for VPrintf; gfx.lib will
				;then make the proper v39 check. 
				;So an error for not being able to 
				;open v39 gfx.lib can be printed 
				;when started e.g. from 2.04/v36.

	jsr	_LVOOpenLibrary(a6)
	move.l	d0,_DOSBase
	beq	Exit

	;
	; Open graphics.library
	;
.openGfxLib
	lea	gfxname(pc),a1
	moveq.l #39,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,_GfxBase
	bne.s	.loadImages
	
	;Failed to open graphics.library
	move.l	#strErrOpenGfx,d1	;Error message string for PrintStdOut
	bsr	PrintStdOut
	bra	Exit

.loadImages

	;
	; Load the background RAW image
	;
	move.l	#BGIMGSIZE,d0	;Size of image to be loaded
	move.l	#bgImgName,d1	;Name of image to be loaded
	bsr	LoadRawImage
	move.l	d0,bgImg	;Save pointer to allocated buffer
	beq	Exit		;Loading failed as d0 is zero

	;
	; Load the RAW image of duck 1
	;
	move.l	#DUCKSIZE,d0		;Size of duck 1 image
	move.l	#duckImg1Name,d1	;Name of duck 1 image file
	bsr	LoadRawImage
	move.l	d0,duckImg1		;Save pointer to allocated buffer
	beq	Exit			;Loading failed as d0 is zero

.saveOldView
	;
	; Switch off current copper list without smashing the current
	; screen. The order of LoadView(0) and 2xWaitTOF() is set by
	; Commodore
	;
	movea.l _GfxBase,a6		;Use graphics.library
	move.l	gb_ActiView(a6),oldview	;Save current view

.clearView
	sub.l	a1,a1			;Clear a1 to display no view
	jsr	_LVOLoadView(a6)
	jsr	_LVOWaitTOF(a6)
	jsr	_LVOWaitTOF(a6)

	cmp.l	#0,gb_ActiView(a6)	; Any other view appeared?
	bne.s	.clearView		; If so wipe it.

	jsr	_LVOOwnBlitter(a6)

	movea.l _SysBase,a6
	jsr	_LVOForbid(a6)

*======================================================================
* Main
*======================================================================
	;
	; Load address of allocated memory for the background bgImg
	; and set the bitplane pointers for all four planes in the
	; in the copper list
	;
	move.l	bgImg(pc),d0
	move.l	#BGIMGPLANESIZE,d1	;The size of one bitplane in bytes
					;640/8*256 = 20480


	move.w	d0,plane1+6
	swap	d0
	move.w	d0,plane1+2

	swap	d0
	add.l	d1,d0

	move.w	d0,plane2+6
	swap	d0
	move.w	d0,plane2+2

	swap	d0
	add.l	d1,d0

	move.w	d0,plane3+6
	swap	d0
	move.w	d0,plane3+2

	swap	d0
	add.l	d1,d0

	move.w	d0,plane4+6
	swap	d0
	move.w	d0,plane4+2

	; Activate the copperlist
	lea	_custom,a1
	move.l	#copperlist,cop1lc(a1)

	; Activate the bitplane and blitter DMA
	move.w	#$8140,dmacon(a1)

Blit	;Blit the duck image
	clr.w	$dff042	 ; Clear BLTCON1
	clr.w	$dff064	 ; Clear BLTAMOD

;	move.w	#$ffff,$dff044	; BLTAFWM -> Blitter first word mask for source A
;	move.w	#$ffff,$dff046	; BLTALWM -> Blitter last word mask for source A

	move.w	#%0000100111110000,$DFF040	; BLTCON0 Enable Minterms Abc, AbC, ABc, ABC, DMA Target D, DMA Source A

	move.w	#(640-64)/8,$DFF066	; BLTDMOD Blitter modulo for destination D
	move.l	duckImg1(pc),$dff050	; BLTAPTH Blitter pointer to source A
	move.l	bgImg(pc),a1		; Load Background image ptr (to be used as blit destination)
	add.l	#16040,a1		; Move the duck to be in front of the hill on bg pic
	moveq.l #4,d4			; Want to copy 5 bitplanes (cycle runs from 4, 3, 2, 1, 0)

.planeLoop
	move.l	a1,$dff054		; BLTDPTH Blitter pointer to destination D (high 5 bits)
	add.l	#$5000,a1		; Increase a1 pointer about 20480 (the size of one plane 640 x 256)
	move.w	#DUCKBLTSIZE,$dff058	; BLTSIZE -> start blitting

.waitBlit: 
	btst	#6,$dff002		; DMACONR -> Test bit 6 - it is set when blitter still working
	bne.s	.waitBlit

	dbf	d4,.planeLoop		; If not -1, continue at 'hier' and copy next plane

Main

.loop	;Wait until left mouse button pressed
	move.l	$dff004,d0		;Wait for the beam (WaitTOF?)
	and.l	#$fff00,d0
	cmp.l	#$00003000,d0
	bne.s	.loop

	btst	#CIAB_GAMEPORT0,_ciaa	;Mouse button pressed
	bne	.loop


*======================================================================
* Clean up
*======================================================================
	movea.l _SysBase,a6
	jsr	_LVOPermit(a6)

	movea.l _GfxBase,a6
	jsr	_LVODisownBlitter(a6)


Exit
	move.l	_GfxBase,a6	 ;Use graphics.library
	move.l	oldview,d0		;Restore former view
	beq	.ex1		;But only if there was one
	jsr	_LVOWaitTOF(a6)
	jsr	_LVOWaitTOF(a6)
	move.l	oldview,a1
	jsr	_LVOLoadView(a6)

.ex1
	; Start former copper list
	lea.l	_custom,a1
	move.l	gb_copinit(a6),cop1lc(a1)

	;
	; Free memory if it was allocated successfully
	;
	movea.l _SysBase,a6	;Use exec.library

	move.l	duckImg1,d0
	beq	.free1		;Image data for duck1 was not allocated
	move.l	d0,a1
	jsr	_LVOFreeVec(a6)

.free1
	move.l	bgImg,d0
	beq	.ex2		;Image data for bgImg was not allocated
	move.l	d0,a1
	jsr	_LVOFreeVec(a6)

.ex2
	; Close graphics.library
	movea.l _SysBase,a6
	move.l	_GfxBase,d0	;Verify: LibBase needed in d-reg
	beq	.ex3
	move.l	d0,a1		;Closing: LibBase needed in a1
	jsr	_LVOCloseLibrary(a6)

.ex3
	; Close dos.library
	movea.l _SysBase,a6
	move.l	_DOSBase,d0	;Verify: LibBase needed in d-reg
	beq	.ex4
	move.l	d0,a1		;Closing: LibBase needed in a1
	jsr	_LVOCloseLibrary(a6)


.ex4
;	move.l	#$8020,$dff096		;Enable sprites
	lea	_custom,a1
	move.w	#$8020,dmacon(a1)
	rts



*======================================================================
* Sub
*======================================================================
; Function LoadRawImage
;	Loads a RAW image file of given size into a buffer of CHIP memory 
;	which it allocates before. Returns the address off allocated memory.
;	When finished the allocated memory must be freed using FreeVec.
;
; Parameters
;	d0: RAW image size in bytes
;	d1: File name of image file
;
; Returns
;	d0: On success: address of reserved memory. On error: 0.
LoadRawImage
	move.l	d0,d3		;Copy of image size
	move.l	d1,d4		;Copy of image name
	
	;
	; Allocate memory for bgImg
	;
				;Size of needed memory is already in d0
	move.l	#MEMF_CHIP,d1	;It must be Chip memory
	or.l	#MEMF_CLEAR,d1	;New memory should be cleared
	jsr	_LVOAllocVec(a6)
	move.l	d0,d5		;Save ptr of allocated memory
	bne.s	.load		;Continue with loading the image
	
	move.l	#strErrAllocChipMem,d1 ;The error message as alloc failed
	bsr	PrintStdOut
	sub.l	d0,d0		;Clear d0 to mark that function failed
	rts			;Return with error

.load
	; Load the Background image at reserved memory address
	move.l	d4,d1		;LoadFileToBuf needs file name in d1
	move.l	d0,d2		;..and buf addr in d2
				;..and buf len/image size in d3, 
				;which has been done at function start
	bsr	LoadFileToBuf
	tst.l	d0
	bne.s	.loadingFailed	;If d0 is not zero, loading failed
	move.l	d5,d0		;Restore pointer of allocated memory to d0
	rts			 ;Return with success

	; Loading of background image failed
.loadingFailed
	move.l	d4,d2		;Move file name address to d2 to also
				;be printed within PrintDosErr

	bsr	PrintDosErr	;Print Dos error message
	sub.l	d0,d0		;Clear d0 to mark that function failed
	rts			;Return with error


; Function LoadFileToBuf
;	Loads a file into a memory buffer which must have been reserved
;	before. Opens the file before and closes it after loading.
;
;	The comment could even go on here. You could write more and more
;	Text..if you wanted to do so..
;
; Parameters
;	d1: file name
;	d2: buf address
;	d3: buf size
;
; Returns
;	d0: zero on success or error code
LoadFileToBuf:
	movem.l d2-d6/a2-a6,-(sp)
	move.l	d2,d4			;Save buf addr for Read

	move.l	_DOSBase,a6

	move.l	#MODE_OLDFILE,d2
	jsr	_LVOOpen(a6)
	tst.l	d0
	beq	.openErr		;d0 = 0, File not opened

	move.l	d0,d5			;Save file handle for Close

	move.l	d0,d1			;But Read needs it in d1
	move.l	d4,d2			;Restore buf addr from d4
	jsr	_LVORead(a6)
	tst.l	d0
	blt	.readErr		;d0 < 0; Read error

	move.l	d5,d1			;Restore file handle from d5
	jsr	_LVOClose(a6)

	movem.l (sp)+,d2-d6/a2-a6
	sub.l	d0,d0			;Success: d0 = 0
	rts

.readErr
	move.l	d5,d1			;Restore file handle from d5
	jsr	_LVOClose(a6)

	movem.l (sp)+,d2-d6/a2-a6
	move.l	#-1,d0			;Error: d0 = error code
	rts


.openErr
	movem.l (sp)+,d2-d6/a2-a6
	move.l	#-2,d0			;Error: d0 = error code
	rts



; Function PrintDosErr
;	Prints a AmigaDos error message on stdout
;
; Parameters
;	d2: Address of file name which was the resaon for the error.
;	Can be zero.
;
PrintDosErr
	move.l	_DOSBase,a6	 	;Use dos.library

	jsr	_LVOIoErr(a6);		;Get error code of last Dos operation
	move.l	d0,d1			;It is needed in d1
	jsr	_LVOPrintFault(a6)	;Print error message
	rts



; Function PrintStdOut
;	Prints a text to stdout
;
; Parameters
;	d1: Address of the string to be printed
;
PrintStdOut
	movea.l _DOSBase,a6
	move.l	0,d2
	jsr	_LVOVPrintf(a6)
	bra	Exit

*======================================================================
* Data
*======================================================================

dosname		DOSNAME		;dos.i
gfxname		GRAPHICSNAME	;gfxbase.i
		even

_DOSBase	ds.l	1
_GfxBase	ds.l	1
_SysBase	ds.l	1

oldview		ds.l	1

bgImg		even
		ds.l	1

duckImg1	even
		ds.l	1

duckImg2	even
		ds.l	1

strErrOpenGfx	even
		dc.b	'Failed to open graphics.library v39.',10,0

strErrAllocChipMem	
		even
		dc.b	'Failed to allocate needed amount of chip memory.',10,0

bgImgName	even
		dc.b	'gfx/background.raw',0

duckImg1Name	even
		dc.b	'gfx/duck1.raw',0

duckImg2Name	even
		dc.b	'gfx/duck2.raw',0


		SECTION "dma",data,chip
		even
copperlist
;		dc.w	$0207,$fffe	;Required for AGA machines
		dc.w	dmacon,$0020 	;Disable sprites
		dc.w	diwstrt,$2C81
		dc.w	diwstop,$2CC1
		dc.w	ddfstrt,$003C
		dc.w	ddfstop,$00d4
		dc.w	bplcon0,$C200	; Hires, 4 bitplanes
		dc.w	bplcon1,$0000
		dc.w	bplcon2,$0000
		dc.w	bpl1mod,$0000
		dc.w	bpl2mod,$0000
plane1		dc.w	bplpt,$0000
		dc.w	bplpt+2,$0000
plane2		dc.w	bplpt+4,$0000
		dc.w	bplpt+6,$0000
plane3		dc.w	bplpt+8,$0000
		dc.w	bplpt+10,$0000
plane4		dc.w	bplpt+12,$0000
		dc.w	bplpt+14,$0000
		dc.w	color+2,$0222	;COLOR01
		dc.w	color+18,$0991	;COLOR09
		dc.w	color+20,$0bb2	;COLOR10
		dc.w	color+24,$0fb2	;COLOR12
;		dc.w	$7007,$fffe	;WAIT
;		dc.w	color,$0fff	;COLOR00 -> white
;		dc.w	$e007,$fffe	;WAIT
;		dc.w	color,$0f00	;COLOR00 -> red
;		dc.w	$ff07,$fffe	;Wait for last ntsc line
;		dc.w	dmacon,$8020	;Enable sprites
		dc.w	$ffff,$fffe	;Waiting for impossible position

	END
