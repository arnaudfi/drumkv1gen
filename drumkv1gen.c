
/*
 * drumk1gen - a tiny tool to generate a mapping file for drumkv1
 * from a folder full of wav files or based on a supplied mapping
 * file
 *
 * Frank Neumann, 2013-2018
 *
 *  This file is part of drumkv1gen, which is free software: you can redistribute
 *  it and/or modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  drumkv1gen is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.   See the GNU General Public License (version 2 or
 *  later  for more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  drumkv1gen; if not, write to the Free Software Foundation, Inc., 51 Franklin
 *  Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *  Created in October 2018
 *
 */


/*
 * Todo:
 * x add hihats choked by default
 * - add optional "plausibility check" (do referenced files exist at all?)
 * - output a .svg file that shows the keymap for the created file
 * - set amp and filter env to "as long as possible" to avoid cutting of samples
 * ? allow user to specify initial key number (currently hardcoded to 0 -> It was 36 in the original pgm : do not manage to make it an option)  
 * - detect re-use of already mapped keys in map file
 * x write man page?
 * x add long options to help text
 * - ideal setting for velocity: ca. 50
 * x print out how many samples were mapped
 * - print out generated file names
 * x handle file names with white space correctly (whole 2nd parameter of line)
 * - "quiet mode" (-q - output nothing on the console)
 * - also support .mp3/.MP3 and .ogg extensions, maybe others (whatever drumkv1 supports)
 */

/*** DEFINES ***/
#define TRUE 1
#define FALSE 0
#define DRUMK1GEN_VERSION "0.2"
#define MAX_SAMPLES 128

/*** INCLUDES ***/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <getopt.h>

/*** PROTOTYPES ***/
int print_help (char *exename);
void print_version(char *progname);
int process_mapfile(int flag_force, char *out_name, char *mapname);
int process_dir(int flag_force, char *in_dirname, char *out_name);
void write_header(FILE *fp1, char *filename);
void write_footer(FILE *fp1);
void write_element(FILE *fp1, int key, char *fullname);

/*                              */
/* print out program usage text */
/*                              */
int print_help(char *exename)
{
	fprintf(stderr, "Usage: %s [options] -d <dir> -n <name>\n", exename);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, " -h, --help                        print usage (this text)\n");
	fprintf(stderr, " -V, --version                     print program version number and exit\n");
	fprintf(stderr, "\n");
	fprintf(stderr, " -d <dir>, --dir <dir>             specifies directory to read sample files from (*.wav, *.WAV)\n");
	fprintf(stderr, " -o <name> , --output <name>       specifies base name of output files (extensions .map and.drumkv1 will be appended)\n");
	fprintf(stderr, " -f, --force                       overwrite existing files without asking\n");
	fprintf(stderr, " -m <mapfile>, --mapfile <mapfile  read mapping from <mapfile>\n");
	exit(0);
}

/*                                  */
/* print out program version number */
/*                                  */
void print_version(char *progname)
{
	fprintf(stderr, "%s version %s\n", progname, DRUMK1GEN_VERSION);
	fprintf(stderr, "Copyright (c) 2013-2018 Frank Neumann\n");
	fprintf(stderr, "This is free software; see the source for copying conditions.\n");
	fprintf(stderr, "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n");
	fprintf(stderr, "PARTICULAR PURPOSE.\n");
	exit(0);
}


/*** string comparison func to qsort() the file list ***/
int mycmpfunc(const void *str1, const void *str2)
{
	return strcmp( *(char *const *)str1, *(char * const *)str2);
}


/*********************/
/*** main function ***/
/*********************/
int main(int argc, char * argv[])
{
	char in_dirname[1024], out_name[1024], mapname[1024];
	char has_dir = FALSE, has_outname = FALSE, has_mapfile = FALSE;
	char flag_force = FALSE;
	int opt_index, getopt_return;
	static struct option long_opts[] =
	{
		/* longoption, has_arg (0, 1, 2), flag, val */
		{ "help", 0, NULL, 'h' },
		{ "version", 0, NULL, 'V' },
		{ "force", 0, NULL, 'f' },
		{ "dir", 1, NULL, 'd' },
		{ "outputfile", 1, NULL, 'o' },
		{ "mapfile", 1, NULL, 'm' },
		{ 0, 0, 0, 0 }
	} ;

	if (argc < 2)
		print_help(argv[0]);

	while((getopt_return = getopt_long(argc, argv, "hVfd:o:m:",
		long_opts, &opt_index)) > 0)
	{
//		fprintf(stderr, "opt char:'%c'\n", getopt_return);
		switch(getopt_return)
		{
			case 'h':
				print_help(argv[0]);
				break;
			case 'V':
				print_version(argv[0]);
				break;
			case 'f':
				flag_force = TRUE;
				break;
			case 'd':
				strcpy(in_dirname, optarg);
				has_dir = TRUE;
				break;
			case 'o':
				strcpy(out_name, optarg);
				has_outname = TRUE;
				break;
			case 'm':
				strcpy(mapname, optarg);
				has_mapfile = TRUE;
				break;

			default:
				fprintf(stderr, "Uncaught option - this is an ERROR.\n");
				exit(5);
		}
	}

	/* we need both input directory and output file name */
	if (!has_outname)
		print_help(argv[0]);

	/* need either -d (input directory) or -m (map file) */
	if (!has_dir && !has_mapfile)
		print_help(argv[0]);

	/* then again, both input directory and map file name together don't make sense */
	if (has_dir && has_mapfile)
		print_help(argv[0]);

	if (has_mapfile)
	{
		process_mapfile(flag_force, out_name, mapname);
	}
	else
		process_dir(flag_force, in_dirname, out_name);

	exit(0);
}


int process_mapfile(int flag_force, char *out_name, char *mapname)
{
	FILE *map_fp, *out_fp;
	int linecnt = 0, keynum;
	char *keystr;
	char linebuf[4096];
	char fname[4096];
	int num_samples = 0;

	map_fp = fopen(mapname, "r");
	if (!map_fp)
	{
		fprintf(stderr, "Unable to open file '%s'\n", mapname);
	}

	/* does output file already exist? */
	if (!access(out_name, F_OK) && !flag_force)
	{
		fprintf(stderr, "File '%s' already exists, and you did not specify --force - aborting.\n", out_name);
		exit(5);
	}

	out_fp = fopen(out_name, "w");
	if (!out_fp)
	{
		fprintf(stderr, "Unable to open output file '%s' for writing - abort.\n", out_name);
		exit(5);
	}

	write_header(out_fp, out_name);

	/* read entries from map file one by one */
	while (fgets(linebuf, 4095, map_fp))
	{
		linecnt++;
//		fprintf(stderr, "line (len=%ld): '%s'\n", strlen(linebuf), linebuf);
		if (strlen(linebuf) == 1)
		{
			/* empty line, do nothing */
		}
		else if (linebuf[0] == '#')
		{
			/* comment line: again do nothing */
			/* TODO: what about indented comment lines? */
		}
		else
		{
			/* attempt to scan 2 elements from line, MIDI note # and file name */
			/* first, note # */
			keystr = strtok(linebuf, " \t");
			if (sscanf(keystr, "%d", &keynum) != 1)
			{
				fprintf(stderr, "Malformed input in map file, line %d\n", linecnt);
				exit(5);
			}
			/* treat everything from here til end of line as the filename.
			 * This allows to also use filenames with white space in them, not uncommon
			 * in sample libraries.
			 */
			strcpy(fname, strtok(NULL, "\n"));
			if (strlen(fname) == 0)
			{
				fprintf(stderr, "Malformed input in map file, line %d\n", linecnt);
				exit(5);
			}

			write_element(out_fp, keynum, fname);
			num_samples++;
		}
	}

	write_footer(out_fp);
	fclose(map_fp);
	fclose(out_fp);

	printf("Created map files '%s' and '%s' with %d real entries each.\n", out_name, "TODO", num_samples);
	return 0;
}


int process_dir(int flag_force, char *in_dirname, char *out_name)
{
	FILE *fp1;
	DIR *dir;
	struct dirent *dirent;
	char *namelist[1024];  /* hard-coded maximum table size for now */
	char fullname[4096];
	int i, num_entries = 0;
	int key = 0;    /* first MIDI key to start with */

	// <dir> must be a directory
	dir = opendir(in_dirname);
	if (dir == NULL)
	{
		fprintf(stderr, "Unable to open directory '%s' for reading.\n", in_dirname);
		exit(5);
	}

	while ( (dirent = readdir(dir)) != NULL)
	{
//		printf("next file: '%s'", dirent->d_name);
		if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0 )
		{
//			printf("..skipping.\n");
			continue;
		}
		if (dirent->d_type != DT_REG && dirent->d_type != DT_LNK)
		{
//			printf("..skipping (not regular file or symlink)\n");
			continue;
		}

		if (strncasecmp(dirent->d_name + strlen(dirent->d_name) - 4, ".wav", 4) != 0)
		{
//			printf("..skipping, does not end in .wav or .WAV.\n");
			continue;
		}

//		printf("have one here: '%s'\n", dirent->d_name);
		namelist[num_entries] = malloc(strlen(dirent->d_name) + 1);
		if (!namelist[num_entries])
		{
			fprintf(stderr, "Error: no memory for string\n");
			exit(5);
		}
		strcpy(namelist[num_entries], dirent->d_name);
		num_entries++;
	}

#if 0
	printf("num_entries: %d\n", num_entries);
	printf("-- before sort: --\n");
	for (i = 0; i < num_entries; i++)
	{
		printf("string #%d: '%s'\n", i, namelist[i]);
	}
#endif

	qsort(namelist, num_entries, sizeof(char *), mycmpfunc);

	/* did we find anything at all? */
	if (num_entries == 0)
	{
		fprintf(stderr, "No samples (*.wav, *.WAV) found in given directory - nothing was written.\n");
		exit(1);
	}

	/* don't attempt to create mappings for more than (127-36+1)=92 samples */
	if (num_entries > MAX_SAMPLES)
	{
		fprintf(stderr, "Directory contains more than %d samples which is\n", MAX_SAMPLES);
		fprintf(stderr, "the current maximum (MIDI note 36..127). No output was created.\n");
		exit(5);
	}

	/* does output file already exist? */
	if (!access(out_name, F_OK) && !flag_force)
	{
		fprintf(stderr, "File '%s' already exists, and you did not specify --force - aborting.\n", out_name);
		exit(5);
	}

	fp1 = fopen(out_name, "w");
	if (!fp1)
	{
		fprintf(stderr, "Unable to open output file '%s' for writing - abort.\n", out_name);
		exit(5);
	}

	write_header(fp1, out_name);

	for (i = 0; i < num_entries; i++)
	{
		/* put together full filename */
		strcpy(fullname, in_dirname);
		strcat(fullname, "/");
		strcat(fullname, namelist[i]);

		write_element(fp1, key++, fullname);
//		printf("string #%d: '%s'\n", i, namelist[i]);
	}

	write_footer(fp1);
	fclose(fp1);

	closedir(dir);
	printf("Created map files '%s' and '%s' with %d real entries each.\n", out_name, "TODO", num_entries);

	return 0;
}


/*                           */
/* write out the file header */
/*                           */
void write_header(FILE *fp1, char *filename)
{
	fprintf(fp1, "<!DOCTYPE drumkv1>\n");
	fprintf(fp1,"<preset version=\"0.9.3\" name=\"%s\">\n", filename);
	fprintf(fp1, " <elements>\n");
}

/*                                             */
/* write out global parameters and file footer */
/*                                             */
void write_footer(FILE *fp1)
{
	fprintf(fp1, " </elements>\n");

	fprintf(fp1, " <params>\n");
	fprintf(fp1, "  <param index=\"42\" name=\"DEF1_PITCHBEND\">0</param>\n");
	fprintf(fp1, "  <param index=\"43\" name=\"DEF1_MODWHEEL\">0</param>\n");
	fprintf(fp1, "  <param index=\"44\" name=\"DEF1_PRESSURE\">0.2</param>\n");
	fprintf(fp1, "  <param index=\"45\" name=\"DEF1_VELOCITY\">0.2</param>\n");
	fprintf(fp1, "  <param index=\"46\" name=\"DEF1_CHANNEL\">0</param>\n");
	fprintf(fp1, "  <param index=\"47\" name=\"DEF1_NOTEOFF\">1</param>\n");
	fprintf(fp1, "  <param index=\"48\" name=\"CHO1_WET\">0</param>\n");
	fprintf(fp1, "  <param index=\"49\" name=\"CHO1_DELAY\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"50\" name=\"CHO1_FEEDB\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"51\" name=\"CHO1_RATE\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"52\" name=\"CHO1_MOD\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"53\" name=\"FLA1_WET\">0</param>\n");
	fprintf(fp1, "  <param index=\"54\" name=\"FLA1_DELAY\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"55\" name=\"FLA1_FEEDB\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"56\" name=\"FLA1_DAFT\">0</param>\n");
	fprintf(fp1, "  <param index=\"57\" name=\"PHA1_WET\">0</param>\n");
	fprintf(fp1, "  <param index=\"58\" name=\"PHA1_RATE\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"59\" name=\"PHA1_FEEDB\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"60\" name=\"PHA1_DEPTH\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"61\" name=\"PHA1_DAFT\">0</param>\n");
	fprintf(fp1, "  <param index=\"62\" name=\"DEL1_WET\">0</param>\n");
	fprintf(fp1, "  <param index=\"63\" name=\"DEL1_DELAY\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"64\" name=\"DEL1_FEEDB\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"65\" name=\"DEL1_BPM\">180</param>\n");
	fprintf(fp1, "  <param index=\"66\" name=\"REV1_WET\">0</param>\n");
	fprintf(fp1, "  <param index=\"67\" name=\"REV1_ROOM\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"68\" name=\"REV1_DAMP\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"69\" name=\"REV1_FEEDB\">0.5</param>\n");
	fprintf(fp1, "  <param index=\"70\" name=\"REV1_WIDTH\">0</param>\n");
	fprintf(fp1, "  <param index=\"71\" name=\"DYN1_COMPRESS\">0</param>\n");
	fprintf(fp1, "  <param index=\"72\" name=\"DYN1_LIMITER\">1</param>\n");
	fprintf(fp1, " </params>\n");
	fprintf(fp1, "</preset>\n");
}

/*                                          */
/* write out a single element (sample, key) */
/*                                          */
void write_element(FILE *fp1, int key, char *fullname)
{
	fprintf(fp1, "  <element index=\"%d\">\n", key);
	/* missing: offset-end=.. */
	fprintf(fp1, "   <sample offset-start=\"0\" index=\"0\" name=\"GEN1_SAMPLE\">%s</sample>\n", fullname);
	fprintf(fp1, "   <params>\n");
	fprintf(fp1, "    <param index=\"0\" name=\"GEN1_SAMPLE\">%d</param>\n", key);
	fprintf(fp1, "    <param index=\"1\" name=\"GEN1_REVERSE\">0</param>\n");
	fprintf(fp1, "    <param index=\"2\" name=\"GEN1_OFFSET\">0</param>\n");
	fprintf(fp1, "    <param index=\"3\" name=\"GEN1_OFFSET_1\">0</param>\n");
	fprintf(fp1, "    <param index=\"4\" name=\"GEN1_OFFSET_2\">0</param>\n");
	if(key==42 || key==44 || key==46){
	fprintf(fp1, "    <param index=\"5\" name=\"GEN1_GROUP\">1</param>\n");
	}
	else{
	fprintf(fp1, "    <param index=\"5\" name=\"GEN1_GROUP\">0</param>\n");
	}
	fprintf(fp1, "    <param index=\"6\" name=\"GEN1_COARSE\">0</param>\n");
	fprintf(fp1, "    <param index=\"7\" name=\"GEN1_FINE\" >0</param>\n");
	fprintf(fp1, "    <param index=\"8\" name=\"GEN1_ENVTIME\">1.0</param>\n");
	fprintf(fp1, "    <param index=\"9\" name=\"DCF1_CUTOFF\">1</param>\n");
	fprintf(fp1, "    <param index=\"10\" name=\"DCF1_RESO\">0</param>\n");
	fprintf(fp1, "    <param index=\"11\" name=\"DCF1_TYPE\">0</param>\n");
	fprintf(fp1, "    <param index=\"12\" name=\"DCF1_SLOPE\">0</param>\n");
	fprintf(fp1, "    <param index=\"13\" name=\"DCF1_ENVELOPE\">1</param>\n");
	fprintf(fp1, "    <param index=\"14\" name=\"DCF1_ATTACK\">0</param>\n");
	fprintf(fp1, "    <param index=\"15\" name=\"DCF1_DECAY1\">0.5</param>\n");
	fprintf(fp1, "    <param index=\"16\" name=\"DCF1_LEVEL2\">0.2</param>\n");
	fprintf(fp1, "    <param index=\"17\" name=\"DCF1_DECAY2\">0.5</param>\n");
	fprintf(fp1, "    <param index=\"18\" name=\"LFO1_SHAPE\">1</param>\n");
	fprintf(fp1, "    <param index=\"19\" name=\"LFO1_WIDTH\">1</param>\n");
	fprintf(fp1, "    <param index=\"20\" name=\"LFO1_BPM\">180</param>\n");
	fprintf(fp1, "    <param index=\"21\" name=\"LFO1_RATE\">0.5</param>\n");
	fprintf(fp1, "    <param index=\"22\" name=\"LFO1_SYNC\">0</param>\n");
	fprintf(fp1, "    <param index=\"23\" name=\"LFO1_SWEEP\">0</param>\n");
	fprintf(fp1, "    <param index=\"24\" name=\"LFO1_PITCH\">0</param>\n");
	fprintf(fp1, "    <param index=\"25\" name=\"LFO1_CUTOFF\">0</param>\n");
	fprintf(fp1, "    <param index=\"26\" name=\"LFO1_RESO\">0</param>\n");
	fprintf(fp1, "    <param index=\"27\" name=\"LFO1_PANNING\">0</param>\n");
	fprintf(fp1, "    <param index=\"28\" name=\"LFO1_VOLUME\">0</param>\n");
	fprintf(fp1, "    <param index=\"29\" name=\"LFO1_ATTACK\">0</param>\n");
	fprintf(fp1, "    <param index=\"30\" name=\"LFO1_DECAY1\">0.5</param>\n");
	fprintf(fp1, "    <param index=\"31\" name=\"LFO1_LEVEL2\">0.2</param>\n");
	fprintf(fp1, "    <param index=\"32\" name=\"LFO1_DECAY2\">0.5</param>\n");
	fprintf(fp1, "    <param index=\"33\" name=\"DCA1_VOLUME\">1</param>\n");
	fprintf(fp1, "    <param index=\"34\" name=\"DCA1_ATTACK\">0</param>\n");
	fprintf(fp1, "    <param index=\"35\" name=\"DCA1_DECAY1\">1</param>\n");
	fprintf(fp1, "    <param index=\"36\" name=\"DCA1_LEVEL2\">1</param>\n");
	fprintf(fp1, "    <param index=\"37\" name=\"DCA1_DECAY2\">0.5</param>\n");
	fprintf(fp1, "    <param index=\"38\" name=\"OUT1_WIDTH\">0</param>\n");
	fprintf(fp1, "    <param index=\"39\" name=\"OUT1_PANNING\">0</param>\n");
	fprintf(fp1, "    <param index=\"40\" name=\"OUT1_FXSEND\">1</param>\n");
	fprintf(fp1, "    <param index=\"41\" name=\"OUT1_VOLUME\">0.5</param>\n");
	fprintf(fp1, "   </params>\n");
	fprintf(fp1, "  </element>\n");
//	printf("Key %d: %s/%s\n", key, path, file);
}

