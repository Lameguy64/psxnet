#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpress.h>

#include "bs_load.h"

void LoadBS (int width, int height, int x, int y, int mode, u_long* bsfile_ptr) {

	/*

	*TINY* BS image loader coded by Lameguy64 of Meido-Tek Productions (2013)

	SYNTAX:
	width, height 	- Size of the BS image (image must be a multiple of 16 for both dimensions)
	x, y			- Framebuffer location on where to draw the decoded image
	mode			- Decode color depth mode (0 - 16-bit, 1 - 24-bit)
	bsfile_ptr		- Pointer to the BS image data to decode

	NOTE:
	The BS image must be at least 24 sectors (49152 bytes) or less in size otherwise, the MDEC
	will freeze at a certain point of decoding. This can be set in MC32 by clicking the Attributes
	button, click the Custom check box, enter 24 sectors for maximum frame size, and then click
	the Variable Frame Size radio button before converting.

	*/

	int		mdec_col;
	int		mdec_cellsize	= (16 + (8 * mode));		// Calculate the size of a cell (16 pixels in 16-bit mode, 24 'odd' pixels in 24-bit mode)
	int		mdec_stripsize	= (mdec_cellsize * height); // Calculate the size for storing several cells in a single strip
	RECT	mdec_rect;

	u_long 	mdec_strip[mdec_stripsize];					// Buffer for one vertical strip of decoded image data
	u_long	mdec_buff[DecDCTBufSize(bsfile_ptr) + 1]; 	// I want to avoid using mallocs (for convenience :>)

	DecDCTReset(0);							// Reset the MDEC chip

	DecDCTvlc(bsfile_ptr, &mdec_buff[0]);	// Decompress DCT data
	DecDCTin(&mdec_buff[0], mode);			// Set MDEC input to point to decompressed DCT data

	// Parameters for uploading the strip of decoded MDEC cells into the framebuffer
	mdec_rect.y = y;
	mdec_rect.w = mdec_cellsize;
	mdec_rect.h = height;

	// This is where the actual decode process takes place
	for (mdec_col = 0; mdec_col < (width / 16); mdec_col += 1) {

		DecDCTout(&mdec_strip[0], mdec_stripsize / 2);	// Grab a strip of decoded image data
		DecDCToutSync(0);								// Wait for the decode to finish

		// Upload the strip into the framebuffer
		mdec_rect.x = x + (mdec_cellsize * mdec_col);
		LoadImage(&mdec_rect, mdec_strip);

		// Wait until the upload has finished before loading another strip
		DrawSync(0);

	}

}
