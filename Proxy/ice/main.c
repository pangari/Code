/*
 * Command-line program for encrypting/decrypting data files
 * with the ICE encryption algorithm.
 *
 * It uses Cipher Block Chaining (CBC) with an initialization
 * vector obtained by default from the gettimeofday call.
 *
 * Usage: ice [-C][-D][-N][-Z][-p passwd][-l level] [filename ...]
 *
 *	-C : Send encrypted/decrypted output to stdout
 *	-D : Decrypt the input data
 *	-N : Do not ask for confirmation for interactive passwords
 *	-Z : Use a zero initializing vector. Otherwise use time of day.
 *	-p : Specify the password on the command-line
 *	-l : Specify the ICE level of encryption
 *
 * If the program is executed with a name beginning with the
 * characters 'de' (e.g. via a symbolic link) the program will carry
 * out decryption by default.
 *
 * For encryption, an encrypted file "infile.ice" will be created for
 * each specified file. It should have the same permission bits as the
 * original files.
 *
 * For decryption, the files must have a ".ice" suffix, or there must
 * be a file "infile.ice" for each file.
 *
 * Written by Matthew Kwan - September 1996
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <fcntl.h>

#include "ice.h"


/*
 * Define boolean types.
 */

typedef unsigned int	BOOL;

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif


/*
 * Maximum password length.
 * Max key is 1024 bytes. 7 bits per char yields 1171 characters.
 */

#define MAX_PASSLEN	1171


/*
 * Build the ICE key from the supplied password.
 * Only uses the lower 7 bits from each character.
 */

static ICE_KEY *
key_build (
	int		level,
	const char	*passwd
) {
	int		i = 0;
	unsigned char	buf[1024];
	ICE_KEY		*ik;

	if ((ik = ice_key_create (level)) == NULL)
	    return (NULL);

	memset (buf, 0, sizeof (buf));

	while (*passwd != '\0') {
	    unsigned char	c = *passwd & 0x7f;
	    int			idx = i / 8;
	    int			bit = i & 7;

	    if (bit == 0) {
		buf[idx] = (c << 1);
	    } else if (bit == 1) {
		buf[idx] |= c;
	    } else {
		buf[idx] |= (c >> (bit - 1));
		buf[idx + 1] = (c << (9 - bit));
	    }

	    i += 7;
	    passwd++;
	}

	ice_key_set (ik, buf);

	return (ik);
}


/*
 * If possible, zero out the contents of a file.
 */

static void
file_zero (
	const char	*fname
) {
	int		fd;
	int		flen, i;
	char		buf[4096];
	struct stat	statbuf;

	if ((fd = open (fname, O_WRONLY)) < 0)
	    return;

	if (fstat (fd, &statbuf) < 0) {
	    close (fd);
	    return;
	}

	for (i=0; i<4096; i++)
	    buf[i] = 0;

	flen = statbuf.st_size;
	for (i=0; i<flen; i+= 4096) {
	    int		n = flen - i;

	    if (n > 4096)
		n = 4096;

	    if (write (fd, buf, n) < 0) {
		close (fd);
		return;
	    }
	}

	close (fd);
}


/*
 * Erase a file.
 * Fill it with zeroes, then unlink it.
 */

static int
erase_file (
	const char	*fname
) {
	file_zero (fname);
	return (unlink (fname));
}


/*
 * Write an 8-byte block to the specified file.
 */

static BOOL
block_write (
	const unsigned char	*buf,
	FILE			*fp
) {
	if (fwrite (buf, sizeof (unsigned char), 8, fp) != 8) {
	    perror (NULL);
	    return (FALSE);
	}

	return (TRUE);
}


/*
 * Copy an 8-byte block.
 */

static void
block_copy (
	unsigned char		*dest,
	const unsigned char	*src
) {
	int		i;

	for (i=0; i<8; i++)
	    dest[i] = src[i];
}


/*
 * XOR an 8-byte block onto another one.
 */

static void
block_xor (
	unsigned char		*dest,
	const unsigned char	*src
) {
	int		i;

	for (i=0; i<8; i++)
	    dest[i] ^= src[i];
}


/*
 * Pad an 8-byte block in positions n .. 7
 * The lower 3 bits of the last byte will contain the value n.
 * The padding will be an encrypted copy of the IV so as to
 * thwart a ciphertext-only attack which might look for padding
 * bytes.
 */

static void
block_pad (
	unsigned char		*buf,
	int			n,
	const unsigned char	*padding
) {
	int		i;

	for (i=n; i<8; i++)
	    buf[i] = padding[i];

	buf[7] = (buf[7] & 0xf8) | n;
}


/*
 * Encrypt a data stream with the given key.
 * Uses cipher-block-chaining mode.
 * The first 3 bytes are the characters "ice". The next character
 * is the character "0" plus the ICE level being used.
 * The next 8 bytes are the initializing vector.
 */

static BOOL
encrypt_stream (
	const ICE_KEY		*ik,
	int			level,
	const unsigned char	*iv,
	FILE			*inf,
	FILE			*outf
) {
	int			n;
	unsigned char		buf[8], inbuf[8], outbuf[8];

	buf[0] = 'i';
	buf[1] = 'c';
	buf[2] = 'e';
	buf[3] = '0' + level;

	if (fwrite (buf, sizeof (unsigned char), 4, outf) != 4) {
	    perror (NULL);
	    return (FALSE);
	}

	block_copy (outbuf, iv);
	if (!block_write (outbuf, outf))
	    return (FALSE);

	while ((n = fread (inbuf, sizeof (unsigned char), 8, inf)) == 8) {
	    block_xor (inbuf, outbuf);
	    ice_key_encrypt (ik, inbuf, outbuf);
	    if (!block_write (outbuf, outf))
		return (FALSE);
	}

	ice_key_encrypt (ik, iv, buf);		/* Generate padding bytes */
	block_pad (inbuf, n, buf);
	block_xor (inbuf, outbuf);
	ice_key_encrypt (ik, inbuf, outbuf);

	if (!block_write (outbuf, outf))
	    return (FALSE);

	return (TRUE);
}


/*
 * Encrypt the specified file.
 * If an output stream isn't specified, write the encrypted data
 * to a file "fname.ice", then remove the original file "fname".
 */

static BOOL
encrypt_file (
	const ICE_KEY		*ik,
	int			level,
	const unsigned char	*iv,
	const char		*fname,
	FILE			*output_file
) {
	FILE			*inf, *outf;
	char			buf[BUFSIZ];
	BOOL			res;

	if ((inf = fopen (fname, "r")) == NULL) {
	    perror (fname);
	    return (FALSE);
	}

	if (output_file == NULL) {
	    struct stat		filestat;

	    sprintf (buf, "%s.ice", fname);
	    if ((outf = fopen (buf, "w")) == NULL) {
		perror (buf);
		fclose (inf);
		return (FALSE);
	    }

			/* Try to maintain same permissions */
	    if (stat (fname, &filestat) == 0)
		chmod (buf, filestat.st_mode);
	} else
	    outf = output_file;

	res = encrypt_stream (ik, level, iv, inf, outf);

	if (output_file == NULL)
	    fclose (outf);

	fclose (inf);

	if (!res)
	    return (FALSE);

			/* Delete the source file if a new file is made */
	if (output_file == NULL && erase_file (fname) < 0) {
	    perror (fname);
	    return (FALSE);
	}

	return (TRUE);
}


/*
 * Decrypt a data stream with the given key.
 * The first 3 characters must be "ice", followed by the encryption
 * level + '0'. Next is the initializing vector, followed by the
 * actual encrypted data.
 * The final block read will be padded. Its true length is given in
 * the lower 3 bits of the 7th byte of the block.
 */

static BOOL
decrypt_stream (
	const char	*passwd,
	FILE		*inf,
	FILE		*outf
) {
	unsigned char	prevbuf[8], inbuf[8], outbuf[8];
	int		n, level;
	ICE_KEY		*ik;
	BOOL		block_loaded_flag;

			/* Check the magic number */
	if (fread (inbuf, sizeof (unsigned char), 4, inf) != 4) {
	    fprintf (stderr, "Could not read magic number\n");
	    return (FALSE);
	}

	if (inbuf[0] != 'i' || inbuf[1] != 'c' || inbuf[2] != 'e') {
	    fprintf (stderr, "Illegal magic number\n");
	    return (FALSE);
	}

	level = inbuf[3] - '0';
	if (level < 0)
	    level += 256;

	if (level > 128) {
	    fprintf (stderr, "Level %d is too high\n", level);
	    return (FALSE);
	}

	if ((ik = key_build (level, passwd)) == NULL)
	    return (FALSE);

			/* Read the initializing vector */
	if (fread (prevbuf, sizeof (unsigned char), 8, inf) != 8) {
	    fprintf (stderr, "Could not read initializing vector\n");
	    return (FALSE);
	}

	block_loaded_flag = FALSE;
	while ((n = fread (inbuf, sizeof (unsigned char), 8, inf)) == 8) {
	    if (block_loaded_flag && !block_write (outbuf, outf))
		return (FALSE);

	    ice_key_decrypt (ik, inbuf, outbuf);
	    block_xor (outbuf, prevbuf);
	    block_loaded_flag = TRUE;

	    block_copy (prevbuf, inbuf);
	}

	if (n != 0)
	    fprintf (stderr, "Warning: truncated final block\n");

		/* The buffer should contain padding info in its last byte */
	n = outbuf[7] & 7;
	if (fwrite (outbuf, sizeof (unsigned char), n, outf) != n) {
	    perror (NULL);
	    return (FALSE);
	}

	ice_key_destroy (ik);

	return (TRUE);
}


/*
 * Decrypt the specified file.
 * If the filename is of the form "foo.ice", that will be the source file.
 * Otherwise, look for a file "filename.ice".
 * If the output stream isn't specified, write to output to a file
 * without the ".ice" suffix, then delete the original file.
 */

static BOOL
decrypt_file (
	const char	*passwd,
	const char	*fname,
	FILE		*output_file
) {
	FILE		*inf, *outf;
	int		n = strlen (fname);
	char		inbuf[BUFSIZ], outbuf[BUFSIZ];
	BOOL		res;

	if (n > 4 && strcmp (&fname[n-4], ".ice") == 0) {
	    strcpy (inbuf, fname);
	    strcpy (outbuf, fname);
	    outbuf[n-4] = '\0';
	} else {
	    sprintf (inbuf, "%s.ice", fname);
	    strcpy (outbuf, fname);
	}

	if ((inf = fopen (inbuf, "r")) == NULL) {
	    perror (inbuf);
	    return (FALSE);
	}
	if (output_file == NULL) {
	    struct stat		filestat;

	    if ((outf = fopen (outbuf, "w")) == NULL) {
		perror (outbuf);
		fclose (inf);
		return (FALSE);
	    }

			/* Try to maintain same permissions */
	    if (stat (inbuf, &filestat) == 0)
		chmod (outbuf, filestat.st_mode);
	} else
	    outf = output_file;

	res = decrypt_stream (passwd, inf, outf);

	if (output_file == NULL)
	    fclose (outf);

	fclose (inf);

	if (!res)
	    return (FALSE);

			/* Delete the source file if a new file is made */
	if (output_file == NULL && erase_file (inbuf) < 0) {
	    perror (inbuf);
	    return (FALSE);
	}

	return (TRUE);
}


/*
 * Prompt the user for a password, with echo disabled.
 * Ask for confirmation if required.
 */

static BOOL
password_read (
	char		*passwd,
	BOOL		confirm_flag
) {
	int		i, fd;
	BOOL		match_ok = TRUE;
	struct termio	t;
	unsigned short	save_lflag;

	if ((fd = open ("/dev/tty", O_RDWR)) < 0) {
	    perror ("/dev/tty");
	    return (FALSE);
	}

	write (fd, "Password:", 9);
	ioctl (fd, TCGETA, &t);
	save_lflag = t.c_lflag;
	t.c_lflag &= ~ECHO;
	ioctl (fd, TCSETAW, &t);

	for (i=0; i<MAX_PASSLEN; i++) {
	    if (read (fd, &passwd[i], 1) != 1)
		break;
	    if (passwd[i] == '\n')
		break;
	}

	passwd[i] = '\0';
	write (fd, "\n", 1);

	if (confirm_flag) {
	    char	buf2[MAX_PASSLEN + 1];

	    write (fd, "Retype password:", 16);
	    for (i=0; i<MAX_PASSLEN; i++) {
		if (read (fd, &buf2[i], 1) != 1)
		    break;
		if (buf2[i] == '\n')
		    break;
	    }

	    buf2[i] = '\0';
	    write (fd, "\n", 1);

	    if (strcmp (passwd, buf2) != 0)
		match_ok = FALSE;
	}

	t.c_lflag = save_lflag;
	ioctl (fd, TCSETAF, &t);
	close (fd);

	if (passwd[0] == '\0') {
	    fprintf (stderr, "No password entered\n");
	    return (FALSE);
	}

	if (match_ok)
	    return (TRUE);

	fprintf (stderr, "Password mismatch\n");
	return (FALSE);
}


/*
 * Build the initial value string.
 * If it isn't zero, set it from the time of day.
 */

static void
build_iv (
	unsigned char	*iv,
	BOOL		zero_flag
) {
	if (zero_flag)
	    memset (iv, 0, 8 * sizeof (unsigned char));
	else {
	    struct timeval	tv;
	    int			i;

	    gettimeofday (&tv, NULL);
	    for (i=0; i<4; i++) {
		iv[i] = tv.tv_sec & 0xff;
		iv[i + 4] = tv.tv_usec & 0xff;

		tv.tv_sec >>= 8;
		tv.tv_usec >>= 8;
	    }
	}
}


/*
 * Fill a string with zeroes.
 * This is used to hide the password argument.
 */

static void
blank_string (
	char	*s
) {
	int	i, n = strlen (s);

	for (i=0; i<n; i++)
	    *s++ = '\0';
}


/*
 * Program's starting point.
 * Processes command-line args and starts things running.
 */

void
main (
	int		argc,
	char		*argv[]
) {
	int		c;
	BOOL		errflag = FALSE;
	BOOL		confirm_flag = TRUE;
	BOOL		decrypt_flag = FALSE;
	BOOL		passwd_flag = FALSE;
	BOOL		zero_iv_flag = FALSE;
	char		passwd[MAX_PASSLEN + 1];
	FILE		*dest_fp = NULL;
	int		level = 1;

	extern char	*optarg;
	extern int	optind;

	while ((c = getopt (argc, argv, "p:l:CDNZ")) != -1)
	    switch (c) {
		case 'C':
		    dest_fp = stdout;
		    break;
		case 'D':
		    decrypt_flag = TRUE;
		    break;
		case 'N':
		    confirm_flag = FALSE;
		    break;
		case 'Z':
		    zero_iv_flag = TRUE;
		    break;
		case 'l':
		    if (sscanf (optarg, "%d", &level) != 1 || level > 128) {
			fprintf (stderr, "Illegal level value '%s'\n", optarg);
			errflag = TRUE;
		    }
		    break;
		case 'p':
		    if (strlen (optarg) > MAX_PASSLEN) {
			fprintf (stderr,
		"Warning: password truncated to %d characters\n", MAX_PASSLEN);
			optarg[MAX_PASSLEN] = '\0';
		    }
		    strcpy (passwd, optarg);
		    blank_string (optarg);
		    passwd_flag = TRUE;

		    break;
		default:
		    errflag = TRUE;
		    break;
	    }

	if (errflag) {
	    fprintf (stderr, "Usage: %s [-C][-D][-N][-Z]", argv[0]);
	    fprintf (stderr, "[-p passwd][-l level] [filename ...]\n");
	    exit (1);
	}

	if (argv[0][0] == 'd' && argv[0][1] == 'e')
	    decrypt_flag = TRUE;

	if (!passwd_flag) {
	    if (!password_read (passwd, confirm_flag))
		exit (1);
	}

	if (decrypt_flag) {
	    if (level != 1)
		fprintf (stderr,
	"Warning: level setting is ignored during decryption\n");

	    if (optind == argc) {
		if (!decrypt_stream (passwd, stdin, stdout))
		    exit (1);
	    } else {
		int	i;

		for (i=optind; i<argc; i++)
		    if (!decrypt_file (passwd, argv[i], dest_fp))
			exit (1);
	    }
	} else {
	    ICE_KEY		*ik;
	    unsigned char	iv[8];

	    if ((ik = key_build (level, passwd)) == NULL)
		exit (1);

	    build_iv (iv, zero_iv_flag);

	    if (optind == argc) {
		if (!encrypt_stream (ik, level, iv, stdin, stdout))
		    exit (1);
	    } else {
		int	i;

		for (i=optind; i<argc; i++)
		    if (!encrypt_file (ik, level, iv, argv[i], dest_fp))
			exit (1);
	    }

	    ice_key_destroy (ik);
	}

	exit (0);
}
