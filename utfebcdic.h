/*    utfebcdic.h
 *
 *    Copyright (C) 2001, 2002, 2003, 2005, 2006, 2007, 2009,
 *    2010, 2011 by Larry Wall, Nick Ing-Simmons, and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * Macros to implement UTF-EBCDIC as perl's internal encoding
 * Taken from version 7.1 of Unicode Technical Report #16:
 *  http://www.unicode.org/unicode/reports/tr16
 *
 * To summarize, the way it works is:
 * To convert an EBCDIC character to UTF-EBCDIC:
 *  1)	convert to Unicode.  The table in this file that does this for
 *	EBCDIC bytes is PL_e2a (with inverse PLa2e).  The 'a' stands for
 *	ASCIIish, meaning latin1.
 *  2)	convert that to a utf8-like string called I8 (I stands for
 *	intermediate) with variant characters occupying multiple bytes.  This
 *	step is similar to the utf8-creating step from Unicode, but the details
 *	are different.  This transformation is called UTF8-Mod.  There is a
 *	chart about the bit patterns in a comment later in this file.  But
 *	essentially here are the differences:
 *			    UTF8		I8
 *	invariant byte	    starts with 0	starts with 0 or 100
 *	continuation byte   starts with 10	starts with 101
 *	start byte	    same in both: if the code point requires N bytes,
 *			    then the leading N bits are 1, followed by a 0.  (No
 *			    trailing 0 for the very largest possible allocation
 *			    in I8, far beyond the current Unicode standard's
 *			    max, as shown in the comment later in this file.)
 *  3)	Use the table published in tr16 to convert each byte from step 2 into
 *	final UTF-EBCDIC.  That table is reproduced in this file as PL_utf2e,
 *	and its inverse is PL_e2utf.  They are constructed so that all EBCDIC
 *	invariants remain invariant, but no others do.  For example, the
 *	ordinal value of 'A' is 193 in EBCDIC, and also is 193 in UTF-EBCDIC.
 *	Step 1) converts it to 65, Step 2 leaves it at 65, and Step 3 converts
 *	it back to 193.  As an example of how a variant character works, take
 *	LATIN SMALL LETTER Y WITH DIAERESIS, which is typically 0xDF in
 *	EBCDIC.  Step 1 converts it to the Unicode value, 0xFF.  Step 2
 *	converts that to two bytes = 11000111 10111111 = C7 BF, and Step 3
 *	converts those to 0x8B 0x73.  The table is constructed so that the
 *	first byte of the final form of a variant will always have its upper
 *	bit set (at least in the encodings that Perl recognizes, and probably
 *	all).  But note that the upper bit of some invariants is also 1.
 * 
 * If you're starting from Unicode, skip step 1.  For UTF-EBCDIC to straight
 * EBCDIC, reverse the steps.
 *
 * The EBCDIC invariants have been chosen to be those characters whose Unicode
 * equivalents have ordinal numbers less than 160, that is the same characters
 * that are expressible in ASCII, plus the C1 controls.  So there are 160
 * invariants instead of the 128 in UTF-8.  (My guess is that this is because
 * the C1 control NEL (and maybe others) is important in IBM.) 
 *
 * The purpose of Step 3 is to make the encoding be invariant for the chosen
 * characters.  This messes up the convenient patterns found in step 2, so
 * generally, one has to undo step 3 into a temporary to use them.  However,
 * a "shadow", or parallel table, PL_utf8skip, has been constructed so that for
 * each byte, it says how long the sequence is if that byte were to begin it 
 *
 * There are actually 3 slightly different UTF-EBCDIC encodings in this file,
 * one for each of the code pages recognized by Perl.  That means that there
 * are actually three different sets of tables, one for each code page.  (If
 * Perl is compiled on platforms using another EBCDIC code page, it may not
 * compile, or Perl may silently mistake it for one of the three.)  
 *
 * EBCDIC characters above 0xFF are the same as Unicode in Perl's
 * implementation of all 3 encodings, so for those Step 1 is trivial.
 *
 * (Note that the entries for invariant characters are necessarily the same in
 * PL_e2a and PLe2f, and the same for their inverses.)
 *
 * UTF-EBCDIC strings are the same length or longer than UTF-8 representations
 * of the same string.  The maximum code point representable as 2 bytes in
 * UTF-EBCDIC is 0x3FFF, instead of 0x7FFF in UTF-8.
 */

START_EXTERN_C

#ifdef DOINIT
/* Indexed by encoded byte this table gives the length of the sequence.
   Adapted from the shadow flags table in tr16.
   The entries marked 9 in tr16 are continuation bytes and are marked
   as length 1 here so that we can recover.
*/
#if '^' == 95   /* if defined(__MVS__) || defined(??) (VM/ESA?) 1047 */
EXTCONST unsigned char PL_utf8skip[] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,2,2,2,2,2,1,1,1,1,1,1,1,
2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,
2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,
2,1,1,1,1,1,1,1,1,1,2,2,2,1,2,2,
2,2,2,2,2,2,2,3,3,3,3,3,3,1,3,3,
1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,
1,1,1,1,1,1,1,1,1,1,3,3,4,4,4,4,
1,4,1,1,1,1,1,1,1,1,4,4,4,5,5,5,
1,1,1,1,1,1,1,1,1,1,5,6,6,7,7,1
};
#endif

#if '^' == 106  /* if defined(_OSD_POSIX) POSIX-BC */
unsigned char PL_utf8skip[] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,2,2,2,2,2,3,1,1,1,1,1,1,
2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,
2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,
2,3,1,1,1,1,1,1,1,1,2,2,2,3,2,2,
1,2,2,2,2,2,2,3,3,3,2,1,1,1,3,3,
4,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,
1,1,1,1,1,1,1,1,1,1,3,3,4,6,4,4,
7,4,1,1,1,1,1,1,1,1,4,4,4,5,5,5,
1,1,1,1,1,1,1,1,1,1,5,1,6,1,7,1
};
#endif

#if '^' == 176  /* if defined(??) (OS/400?) 037 */
unsigned char PL_utf8skip[] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,2,2,2,2,2,1,1,1,1,1,1,1,
2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,
2,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,
2,1,1,1,1,1,1,1,1,1,2,2,2,3,2,2,
1,2,2,2,2,2,2,3,3,3,1,1,3,3,3,3,
1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,
1,1,1,1,1,1,1,1,1,1,3,3,4,4,4,4,
1,4,1,1,1,1,1,1,1,1,4,4,4,5,5,5,
1,1,1,1,1,1,1,1,1,1,5,6,6,7,7,1
};
#endif

/* Transform tables from tr16 applied after encoding to render encoding EBCDIC
 * like, meaning that all the invariants are actually invariant, eg, that 'A'
 * remains 'A' */

#if '^' == 95   /* if defined(__MVS__) || defined(??) (VM/ESA?) 1047 */
EXTCONST unsigned char PL_utf2e[] = { /* I8 to UTFEBCDIC (IBM-1047) */
 0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
 0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
 0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
 0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,
 0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0xFF,
 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
 0x57, 0x58, 0x59, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x70, 0x71, 0x72, 0x73,
 0x74, 0x75, 0x76, 0x77, 0x78, 0x80, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x9A, 0x9B, 0x9C,
 0x9D, 0x9E, 0x9F, 0xA0, 0xAA, 0xAB, 0xAC, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBE, 0xBF, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xDA, 0xDB,
 0xDC, 0xDD, 0xDE, 0xDF, 0xE1, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE
};

EXTCONST unsigned char PL_e2utf[] = { /* UTFEBCDIC (IBM-1047) to I8 */
 0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x9D, 0x0A, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
 0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
 0x20, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
 0x26, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
 0x2D, 0x2F, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
 0xC5, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB,
 0xCC, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2,
 0xD3, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xD4, 0xD5, 0xD6, 0x5B, 0xD7, 0xD8,
 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0x5D, 0xE6, 0xE7,
 0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED,
 0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3,
 0x5C, 0xF4, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,
 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0x9F
};
#endif /* 1047 */

#if '^' == 106  /* if defined(_OSD_POSIX) POSIX-BC */
unsigned char PL_utf2e[] = { /* I8 to UTFEBCDIC (POSIX-BC) */
 0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
 0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
 0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xBB, 0xBC, 0xBD, 0x6A, 0x6D,
 0x4A, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xFB, 0x4F, 0xFD, 0xFF, 0x07,
 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,
 0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0x5F,
 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xB0, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
 0x57, 0x58, 0x59, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xD0, 0x70, 0x71, 0x72, 0x73,
 0x74, 0x75, 0x76, 0x77, 0x78, 0x80, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x9A, 0x9B, 0x9C,
 0x9D, 0x9E, 0x9F, 0xA0, 0xAA, 0xAB, 0xAC, 0xAE, 0xAF, 0xBA, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
 0xB7, 0xB8, 0xB9, 0xAD, 0x79, 0xA1, 0xBE, 0xBF, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xDA, 0xDB,
 0xDC, 0xC0, 0xDE, 0xDF, 0xE1, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xFA, 0xDD, 0xFC, 0xE0, 0xFE
};

unsigned char PL_e2utf[] = { /* UTFEBCDIC (POSIX-BC) to I8 */
 0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x9D, 0x0A, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
 0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
 0x20, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0x60, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
 0x26, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x9F,
 0x2D, 0x2F, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0x5E, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xE4, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
 0xC5, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB,
 0xCC, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2,
 0xD3, 0xE5, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xD4, 0xD5, 0xD6, 0xE3, 0xD7, 0xD8,
 0xA9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xD9, 0x5B, 0x5C, 0x5D, 0xE6, 0xE7,
 0xF1, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED,
 0xBB, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xEE, 0xEF, 0xF0, 0xFC, 0xF2, 0xF3,
 0xFE, 0xF4, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,
 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xFB, 0x7B, 0xFD, 0x7D, 0xFF, 0x7E
};
#endif /* POSIX-BC */

#if '^' == 176  /* if defined(??) (OS/400?) 037 */
unsigned char PL_utf2e[] = { /* I8 to UTFEBCDIC (IBM-037) */
 0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
 0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
 0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xBA, 0xE0, 0xBB, 0xB0, 0x6D,
 0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
 0x20, 0x21, 0x22, 0x23, 0x24, 0x15, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,
 0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0xFF,
 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
 0x57, 0x58, 0x59, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x70, 0x71, 0x72, 0x73,
 0x74, 0x75, 0x76, 0x77, 0x78, 0x80, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x9A, 0x9B, 0x9C,
 0x9D, 0x9E, 0x9F, 0xA0, 0xAA, 0xAB, 0xAC, 0xAE, 0xAF, 0x5F, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
 0xB7, 0xB8, 0xB9, 0xAD, 0xBD, 0xBC, 0xBE, 0xBF, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xDA, 0xDB,
 0xDC, 0xDD, 0xDE, 0xDF, 0xE1, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE
};

unsigned char PL_e2utf[] = { /* UTFEBCDIC (IBM-037) to I8 */
 0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x9D, 0x85, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
 0x80, 0x81, 0x82, 0x83, 0x84, 0x0A, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
 0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
 0x20, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
 0x26, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0xD9,
 0x2D, 0x2F, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
 0xC5, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB,
 0xCC, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2,
 0xD3, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xD4, 0xD5, 0xD6, 0xE3, 0xD7, 0xD8,
 0x5E, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0x5B, 0x5D, 0xE5, 0xE4, 0xE6, 0xE7,
 0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED,
 0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3,
 0x5C, 0xF4, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,
 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0x9F
};
#endif          /* 037 */

/* These tables moved from perl.h and converted to hex.
   They map platform code page from/to bottom 256 codes of Unicode (i.e. iso-8859-1).
*/

#if '^' == 95   /* if defined(__MVS__) || defined(??) (VM/ESA?) 1047 */
EXTCONST unsigned char PL_a2e[] = { /* ASCII (iso-8859-1) to EBCDIC (IBM-1047) */
 0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
 0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
 0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
 0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,
 0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0xFF,
 0x41, 0xAA, 0x4A, 0xB1, 0x9F, 0xB2, 0x6A, 0xB5, 0xBB, 0xB4, 0x9A, 0x8A, 0xB0, 0xCA, 0xAF, 0xBC,
 0x90, 0x8F, 0xEA, 0xFA, 0xBE, 0xA0, 0xB6, 0xB3, 0x9D, 0xDA, 0x9B, 0x8B, 0xB7, 0xB8, 0xB9, 0xAB,
 0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9E, 0x68, 0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,
 0xAC, 0x69, 0xED, 0xEE, 0xEB, 0xEF, 0xEC, 0xBF, 0x80, 0xFD, 0xFE, 0xFB, 0xFC, 0xBA, 0xAE, 0x59,
 0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9C, 0x48, 0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
 0x8C, 0x49, 0xCD, 0xCE, 0xCB, 0xCF, 0xCC, 0xE1, 0x70, 0xDD, 0xDE, 0xDB, 0xDC, 0x8D, 0x8E, 0xDF
};

EXTCONST unsigned char PL_e2a[] = { /* EBCDIC (IBM-1047) to ASCII (iso-8859-1) */
 0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x9D, 0x0A, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
 0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
 0x20, 0xA0, 0xE2, 0xE4, 0xE0, 0xE1, 0xE3, 0xE5, 0xE7, 0xF1, 0xA2, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
 0x26, 0xE9, 0xEA, 0xEB, 0xE8, 0xED, 0xEE, 0xEF, 0xEC, 0xDF, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
 0x2D, 0x2F, 0xC2, 0xC4, 0xC0, 0xC1, 0xC3, 0xC5, 0xC7, 0xD1, 0xA6, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
 0xF8, 0xC9, 0xCA, 0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
 0xD8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xAB, 0xBB, 0xF0, 0xFD, 0xFE, 0xB1,
 0xB0, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xAA, 0xBA, 0xE6, 0xB8, 0xC6, 0xA4,
 0xB5, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xA1, 0xBF, 0xD0, 0x5B, 0xDE, 0xAE,
 0xAC, 0xA3, 0xA5, 0xB7, 0xA9, 0xA7, 0xB6, 0xBC, 0xBD, 0xBE, 0xDD, 0xA8, 0xAF, 0x5D, 0xB4, 0xD7,
 0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xAD, 0xF4, 0xF6, 0xF2, 0xF3, 0xF5,
 0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xB9, 0xFB, 0xFC, 0xF9, 0xFA, 0xFF,
 0x5C, 0xF7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xB2, 0xD4, 0xD6, 0xD2, 0xD3, 0xD5,
 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xB3, 0xDB, 0xDC, 0xD9, 0xDA, 0x9F
};

EXTCONST unsigned char PL_fold[] = { /* fast EBCDIC case folding table, 'A' =>
					'a'; 'a' => 'A' */
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	65,	98,	99,	100,	101,	102,	103,
	104,	105,	74,	75,	76,	77,	78,	79,
	80,	113,	114,	115,	116,	117,	118,	119,
	120,	89,	90,	91,	92,	93,	94,	95,
	96,	97,	66,	67,	68,	69,	70,	71,
	72,	73,	106,	107,	108,	109,	110,	111,
	128,	81,	82,	83,	84,	85,	86,	87,
	88,	121,	122,	123,	124,	125,	126,	127,
	112,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	138,	139,	172,	186,	174,	143,
	144,	'J',	'K',	'L',	'M',	'N',	'O',	'P',
	'Q',	'R',	154,	155,	158,	157,	156,	159,
	160,	161,	'S',	'T',	'U',	'V',	'W',	'X',
	'Y',	'Z',	170,	171,	140,	173,	142,	175,
	176,	177,	178,	179,	180,	181,	182,	183,
	184,	185,	141,	187,	188,	189,	190,	191,
	192,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	202,	235,	236,	237,	238,	239,
	208,	'j',	'k',	'l',	'm',	'n',	'o',	'p',
	'q',	'r',	218,	251,	252,	253,	254,	223,
	224,	225,	's',	't',	'u',	'v',	'w',	'x',
	'y',	'z',	234,	203,	204,	205,	206,	207,
	240,	241,	242,	243,	244,	245,	246,	247,
	248,	249,	250,	219,	220,	221,	222,	255
};
#endif /* 1047 */

#if '^' == 106  /* if defined(_OSD_POSIX) POSIX-BC */
EXTCONST unsigned char PL_a2e[] = { /* ASCII (ISO8859-1) to EBCDIC (POSIX-BC) */
 0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
 0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
 0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xBB, 0xBC, 0xBD, 0x6A, 0x6D,
 0x4A, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xFB, 0x4F, 0xFD, 0xFF, 0x07,
 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,
 0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0x5F,
 0x41, 0xAA, 0xB0, 0xB1, 0x9F, 0xB2, 0xD0, 0xB5, 0x79, 0xB4, 0x9A, 0x8A, 0xBA, 0xCA, 0xAF, 0xA1,
 0x90, 0x8F, 0xEA, 0xFA, 0xBE, 0xA0, 0xB6, 0xB3, 0x9D, 0xDA, 0x9B, 0x8B, 0xB7, 0xB8, 0xB9, 0xAB,
 0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9E, 0x68, 0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,
 0xAC, 0x69, 0xED, 0xEE, 0xEB, 0xEF, 0xEC, 0xBF, 0x80, 0xE0, 0xFE, 0xDD, 0xFC, 0xAD, 0xAE, 0x59,
 0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9C, 0x48, 0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
 0x8C, 0x49, 0xCD, 0xCE, 0xCB, 0xCF, 0xCC, 0xE1, 0x70, 0xC0, 0xDE, 0xDB, 0xDC, 0x8D, 0x8E, 0xDF
};

EXTCONST unsigned char PL_e2a[] = { /* EBCDIC (POSIX-BC) to ASCII (ISO8859-1) */
 0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x9D, 0x0A, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
 0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
 0x20, 0xA0, 0xE2, 0xE4, 0xE0, 0xE1, 0xE3, 0xE5, 0xE7, 0xF1, 0x60, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
 0x26, 0xE9, 0xEA, 0xEB, 0xE8, 0xED, 0xEE, 0xEF, 0xEC, 0xDF, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x9F,
 0x2D, 0x2F, 0xC2, 0xC4, 0xC0, 0xC1, 0xC3, 0xC5, 0xC7, 0xD1, 0x5E, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
 0xF8, 0xC9, 0xCA, 0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0xA8, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
 0xD8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xAB, 0xBB, 0xF0, 0xFD, 0xFE, 0xB1,
 0xB0, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xAA, 0xBA, 0xE6, 0xB8, 0xC6, 0xA4,
 0xB5, 0xAF, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xA1, 0xBF, 0xD0, 0xDD, 0xDE, 0xAE,
 0xA2, 0xA3, 0xA5, 0xB7, 0xA9, 0xA7, 0xB6, 0xBC, 0xBD, 0xBE, 0xAC, 0x5B, 0x5C, 0x5D, 0xB4, 0xD7,
 0xF9, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xAD, 0xF4, 0xF6, 0xF2, 0xF3, 0xF5,
 0xA6, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xB9, 0xFB, 0xFC, 0xDB, 0xFA, 0xFF,
 0xD9, 0xF7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xB2, 0xD4, 0xD6, 0xD2, 0xD3, 0xD5,
 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xB3, 0x7B, 0xDC, 0x7D, 0xDA, 0x7E
};

EXTCONST unsigned char PL_fold[] = { /* fast EBCDIC case folding table, 'A' =>
					'a'; 'a' => 'A' */
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	65,	98,	99,	100,	101,	102,	103,
	104,	105,	74,	75,	76,	77,	78,	79,
	80,	113,	114,	115,	116,	117,	118,	119,
	120,	89,	90,	91,	92,	93,	94,	95,
	96,	97,	66,	67,	68,	69,	70,	71,
	72,	73,	106,	107,	108,	109,	110,	111,
	128,	81,	82,	83,	84,	85,	86,	87,
	88,	121,	122,	123,	124,	125,	126,	127,
	112,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	138,	139,	172,	173,	174,	143,
	144,	'J',	'K',	'L',	'M',	'N',	'O',	'P',
	'Q',	'R',	154,	155,	158,	157,	156,	159,
	160,	161,	'S',	'T',	'U',	'V',	'W',	'X',
	'Y',	'Z',	170,	171,	140,	141,	142,	175,
	176,	177,	178,	179,	180,	181,	182,	183,
	184,	185,	186,	187,	188,	189,	190,	191,
	224,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	202,	235,	236,	237,	238,	239,
	208,	'j',	'k',	'l',	'm',	'n',	'o',	'p',
	'q',	'r',	218,	221,	252,	219,	254,	223,
	192,	225,	's',	't',	'u',	'v',	'w',	'x',
	'y',	'z',	234,	203,	204,	205,	206,	207,
	240,	241,	242,	243,	244,	245,	246,	247,
	248,	249,	250,	251,	220,	253,	222,	255
};
#endif          /* POSIX-BC */

#if '^' == 176  /* if defined(??) (OS/400?) 037 */
EXTCONST unsigned char PL_a2e[] = { /* ASCII (ISO8859-1) to EBCDIC (IBM-037) */
 0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,
 0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
 0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
 0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xBA, 0xE0, 0xBB, 0xB0, 0x6D,
 0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
 0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
 0x20, 0x21, 0x22, 0x23, 0x24, 0x15, 0x06, 0x17, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,
 0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08, 0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0xFF,
 0x41, 0xAA, 0x4A, 0xB1, 0x9F, 0xB2, 0x6A, 0xB5, 0xBD, 0xB4, 0x9A, 0x8A, 0x5F, 0xCA, 0xAF, 0xBC,
 0x90, 0x8F, 0xEA, 0xFA, 0xBE, 0xA0, 0xB6, 0xB3, 0x9D, 0xDA, 0x9B, 0x8B, 0xB7, 0xB8, 0xB9, 0xAB,
 0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9E, 0x68, 0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,
 0xAC, 0x69, 0xED, 0xEE, 0xEB, 0xEF, 0xEC, 0xBF, 0x80, 0xFD, 0xFE, 0xFB, 0xFC, 0xAD, 0xAE, 0x59,
 0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9C, 0x48, 0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
 0x8C, 0x49, 0xCD, 0xCE, 0xCB, 0xCF, 0xCC, 0xE1, 0x70, 0xDD, 0xDE, 0xDB, 0xDC, 0x8D, 0x8E, 0xDF
};

EXTCONST unsigned char PL_e2a[] = { /* EBCDIC (IBM-037) to ASCII (ISO8859-1) */
 0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F, 0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x9D, 0x85, 0x08, 0x87, 0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,
 0x80, 0x81, 0x82, 0x83, 0x84, 0x0A, 0x17, 0x1B, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,
 0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, 0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,
 0x20, 0xA0, 0xE2, 0xE4, 0xE0, 0xE1, 0xE3, 0xE5, 0xE7, 0xF1, 0xA2, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
 0x26, 0xE9, 0xEA, 0xEB, 0xE8, 0xED, 0xEE, 0xEF, 0xEC, 0xDF, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0xAC,
 0x2D, 0x2F, 0xC2, 0xC4, 0xC0, 0xC1, 0xC3, 0xC5, 0xC7, 0xD1, 0xA6, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
 0xF8, 0xC9, 0xCA, 0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
 0xD8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xAB, 0xBB, 0xF0, 0xFD, 0xFE, 0xB1,
 0xB0, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0xAA, 0xBA, 0xE6, 0xB8, 0xC6, 0xA4,
 0xB5, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xA1, 0xBF, 0xD0, 0xDD, 0xDE, 0xAE,
 0x5E, 0xA3, 0xA5, 0xB7, 0xA9, 0xA7, 0xB6, 0xBC, 0xBD, 0xBE, 0x5B, 0x5D, 0xAF, 0xA8, 0xB4, 0xD7,
 0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xAD, 0xF4, 0xF6, 0xF2, 0xF3, 0xF5,
 0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xB9, 0xFB, 0xFC, 0xF9, 0xFA, 0xFF,
 0x5C, 0xF7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xB2, 0xD4, 0xD6, 0xD2, 0xD3, 0xD5,
 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xB3, 0xDB, 0xDC, 0xD9, 0xDA, 0x9F
};

EXTCONST unsigned char PL_fold[] = { /* fast EBCDIC case folding table, 'A' =>
					'a'; 'a' => 'A' */
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,
	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,
	56,	57,	58,	59,	60,	61,	62,	63,
	64,	65,	98,	99,	100,	101,	102,	103,
	104,	105,	74,	75,	76,	77,	78,	79,
	80,	113,	114,	115,	116,	117,	118,	119,
	120,	89,	90,	91,	92,	93,	94,	95,
	96,	97,	66,	67,	68,	69,	70,	71,
	72,	73,	106,	107,	108,	109,	110,	111,
	128,	81,	82,	83,	84,	85,	86,	87,
	88,	121,	122,	123,	124,	125,	126,	127,
	112,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
	'H',	'I',	138,	139,	172,	173,	174,	143,
	144,	'J',	'K',	'L',	'M',	'N',	'O',	'P',
	'Q',	'R',	154,	155,	158,	157,	156,	159,
	160,	161,	'S',	'T',	'U',	'V',	'W',	'X',
	'Y',	'Z',	170,	171,	140,	141,	142,	175,
	176,	177,	178,	179,	180,	181,	182,	183,
	184,	185,	186,	187,	188,	189,	190,	191,
	192,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
	'h',	'i',	202,	235,	236,	237,	238,	239,
	208,	'j',	'k',	'l',	'm',	'n',	'o',	'p',
	'q',	'r',	218,	251,	252,	253,	254,	223,
	224,	225,	's',	't',	'u',	'v',	'w',	'x',
	'y',	'z',	234,	203,	204,	205,	206,	207,
	240,	241,	242,	243,	244,	245,	246,	247,
	248,	249,	250,	219,	220,	221,	222,	255
};
#endif          /* 037 */

/* Since the EBCDIC code pages are isomorphic to Latin1, that table is merely a
 * duplicate */
EXTCONST unsigned char * PL_fold_latin1 = PL_fold;

#else
EXTCONST unsigned char PL_utf8skip[];
EXTCONST unsigned char PL_e2utf[];
EXTCONST unsigned char PL_utf2e[];
EXTCONST unsigned char PL_e2a[];
EXTCONST unsigned char PL_a2e[];
EXTCONST unsigned char PL_fold[];
EXTCONST unsigned char * PL_fold_latin1;
#endif

END_EXTERN_C

/* EBCDIC-happy ways of converting native code to UTF-8 */

#define NATIVE_TO_LATIN1(ch)            PL_e2a[(U8)(ch)]
#define LATIN1_TO_NATIVE(ch)            PL_a2e[(U8)(ch)]

#define NATIVE_UTF8_TO_I8(ch) (ch)      PL_e2utf[(U8)(ch)]
#define I8_TO_NATIVE_UTF8(ch) (ch)      PL_utf2e[(U8)(ch)]

/* Transforms in wide UV chars */
#define NATIVE_TO_UNI(ch)        (((ch) > 255) ? (ch) : NATIVE_TO_LATIN1(ch))
#define UNI_TO_NATIVE(ch)        (((ch) > 255) ? (ch) : LATIN1_TO_NATIVE(ch))

/* Transform in invariant..byte space */
#define NATIVE_TO_NEED(enc,ch)   ((enc)                                     \
                                  ? I8_TO_NATIVE_UTF8(NATIVE_TO_LATIN1(ch)) \
                                  : (ch))
#define ASCII_TO_NEED(enc,ch)    ((enc)                   \
                                  ? I8_TO_NATIVE_UTF8(ch) \
                                  : LATIN1_TO_NATIVE(ch))

/*
  The following table is adapted from tr16, it shows I8 encoding of Unicode code points.

        Unicode                             Bit pattern 1st Byte 2nd Byte 3rd Byte 4th Byte 5th Byte 6th Byte 7th byte
    U+0000..U+007F                     000000000xxxxxxx 0xxxxxxx
    U+0080..U+009F                     00000000100xxxxx 100xxxxx
    U+00A0..U+03FF                     000000yyyyyxxxxx 110yyyyy 101xxxxx
    U+0400..U+3FFF                     00zzzzyyyyyxxxxx 1110zzzz 101yyyyy 101xxxxx
    U+4000..U+3FFFF                 0wwwzzzzzyyyyyxxxxx 11110www 101zzzzz 101yyyyy 101xxxxx
   U+40000..U+3FFFFF            0vvwwwwwzzzzzyyyyyxxxxx 111110vv 101wwwww 101zzzzz 101yyyyy 101xxxxx
  U+400000..U+3FFFFFF       0uvvvvvwwwwwzzzzzyyyyyxxxxx 1111110u 101vvvvv 101wwwww 101zzzzz 101yyyyy 101xxxxx
 U+4000000..U+7FFFFFFF 0tuuuuuvvvvvwwwwwzzzzzyyyyyxxxxx 1111111t 101uuuuu 101vvvvv 101wwwww 101zzzzz 101yyyyy 101xxxxx

  Note: The I8 transformation is valid for UCS-4 values X'0' to
  X'7FFFFFFF' (the full extent of ISO/IEC 10646 coding space).

 */

#define UNISKIP(uv) ( (uv) < 0xA0           ? 1 : \
		      (uv) < 0x400          ? 2 : \
		      (uv) < 0x4000         ? 3 : \
		      (uv) < 0x40000        ? 4 : \
		      (uv) < 0x400000       ? 5 : \
		      (uv) < 0x4000000      ? 6 : 7 )

#define UNI_IS_INVARIANT(c)		((c) <  0xA0)

/* UTF-EBCDIC semantic macros - transform back into I8 and then compare
 * Comments as to the meaning of each are given at their corresponding utf8.h
 * definitions */

#define UTF8_IS_START(c)		(NATIVE_UTF8_TO_I8(c) >= 0xC5     \
                                         && NATIVE_UTF8_TO_I8(c) != 0xE0)
#define UTF8_IS_CONTINUATION(c)		((NATIVE_UTF8_TO_I8(c) & 0xE0) == 0xA0)
#define UTF8_IS_CONTINUED(c) 		(NATIVE_UTF8_TO_I8(c) >= 0xA0)
#define UTF8_IS_DOWNGRADEABLE_START(c)	(NATIVE_UTF8_TO_I8(c) >= 0xC5     \
                                         && NATIVE_UTF8_TO_I8(c) <= 0xC7)
#define UTF8_IS_ABOVE_LATIN1(c)	(NATIVE_UTF8_TO_I8(c) >= 0xC8)

#define UTF_START_MARK(len) (((len) >  7) ? 0xFF : ((U8)(0xFE << (7-(len)))))
#define UTF_START_MASK(len) (((len) >= 6) ? 0x01 : (0x1F >> ((len)-2)))
#define UTF_CONTINUATION_MARK		0xA0
#define UTF_CONTINUATION_MASK		((U8)0x1f)
#define UTF_ACCUMULATION_SHIFT		5

/* How wide can a single UTF-8 encoded character become in bytes. */
/* NOTE: Strictly speaking Perl's UTF-8 should not be called UTF-8 since UTF-8
 * is an encoding of Unicode, and Unicode's upper limit, 0x10FFFF, can be
 * expressed with 5 bytes.  However, Perl thinks of UTF-8 as a way to encode
 * non-negative integers in a binary format, even those above Unicode */
#define UTF8_MAXBYTES 7

/* The maximum number of UTF-8 bytes a single Unicode character can
 * uppercase/lowercase/fold into.  Unicode guarantees that the maximum
 * expansion is 3 characters.  On EBCDIC platforms, the highest Unicode
 * character occupies 5 bytes, therefore this number is 15 */
#define UTF8_MAXBYTES_CASE	15

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 et:
 */
