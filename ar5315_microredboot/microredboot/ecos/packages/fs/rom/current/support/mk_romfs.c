//==========================================================================
//
//      mk_romfs.c
//
//      Create ROM file system image
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           richard.panton@3glab.com
// Contributors:        richard.panton@3glab.com
// Date:                2000-07-25
// Purpose:             ROM file system
// Description:         This program creates a ROM file system image, suitable
//                      for use with the sample ROM file system implemented by
//                      this package.
//                      * CAUTION! * This is host code and can only be built
//                      in a host, e.g. Linux, environment.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//==========================================================================
//
// CONFIGURABLE ITEMS HERE
//
//==========================================================================

// define LONG to be a four byte unsigned integer on the host
#define LONG	unsigned long

// define SHORT to be a two byte unsigned integer on the host
#define SHORT	unsigned short

// All data files should be aligned to this sized boundary (minimum probably 32)
#define DATA_ALIGN	32

// The data stored in a directory should be aligned to this size boundary
#define DIRECTORY_ALIGN	32

// All executable files should be aligned to this sized boundary (minimum probably 32)
#define EXEC_ALIGN	32

// Undefine this if the host filesystem does not support lstat()
#define HAS_LSTAT

//==========================================================================

// Return (n) aligned to the next (b) byte boundary
#define ALIGN_TO( n, b ) (( (n) + (b)-1 ) & ~((b)-1))

// Set the stat call to use
#ifdef HAS_LSTAT
#define get_status( p, b )	lstat( (p), (b) )
#else
#define get_status( p, b )	stat( (p), (b) )
#endif

// This is how we identify a directory from its mode
#define IS_DIRECTORY( m )	(S_ISDIR(m))

// This is how we identify a data file from its mode
#define IS_DATAFILE( m )	(S_ISREG(m) && ((m)&S_IXUSR) == 0 )

// This is how we identify an executable from its mode
#define IS_EXECUTABLE( m )	(S_ISREG(m) && ((m)&S_IXUSR) != 0 )

// This is how we identify a symbolic link from its mode
#define IS_SYMLINK( m )		(S_ISLNK(m))

#define ROMFS_MAGIC	0x526f6d2e

//=========================================================================
// EXIT CODES
#define EXIT_SUCCESS	0
#define EXIT_ARGS	1
#define EXIT_MALLOC	2
#define EXIT_FILESYS	3
#define EXIT_WRITE	4
#define EXIT_SEEK	5
#define EXIT_COMPILE	6
#define EXIT_BUG	7



// These are the structures we will build into the ROMFS image.
// The sizes of these structures should be fixed for all architectures
typedef struct romfs_dirent {
    LONG	node;		// 4
    LONG	next;		// 8
    char	name[0];	// 8 + strlen(name) + 1
} romfs_dirent;			// Aligns to next 32 byte boundary

typedef struct romfs_node {
    LONG	mode;		// 4
    LONG	nlink;		// 8
    SHORT	uid;		// 10
    SHORT	gid;		// 12
    LONG	size;		// 16
    LONG	ctime;		// 20
    LONG	data_offset;	// 24
    char	pad[8];		// 32
} romfs_node;			// Next node begins here

typedef struct romfs_disk {
    LONG	magic;		// 4
    LONG	nodecount;	// 8
    LONG	disksize;	// 12
    LONG	dev_id;		// 16
    char	name[16];	// 32
} romfs_disk;			// Nodes start here

// This is the holding structure for a node
typedef struct node {
    const char *path;		// Filename (inc. path) of a link to this node
    size_t size;		// Size of file/directory/link
    mode_t st_mode;		// Type and permissions
    uid_t uid;			// Owner id
    gid_t gid;			// Group id
    time_t ctime;		// File creation time
    int nodenum;		// Nodenumber of this node in the ROMFS image
    dev_t device;		// Device (for hardlink check)
    ino_t inode;		// Inode (for hardlink check)
    int nlink;			// [DIRECTORIES] Number of sub-directories [FILES] hard links
    romfs_dirent *entry;	// [DIRECTORIES] Points to an array of directory entries
    int entry_size;		// Size to be allocated to file (includes alignment bytes)
    unsigned long offset;	// Offset within ROMFS image of data
    struct node *sibling;	// Points to the next entry in this directory
    struct node *child;		// [DIRECTORIES] Points to any subdirectories
    struct node *next_in_rom;	// Next in ROMFS write order
    struct node *next_multilink;// Next node that is multilinked
} node;

static int nodes = 0;
static char *prog;
static int verbose = 1;
static int dowrite = 1;
static int bigendian = 0;
static int hardlinks = 0;
static unsigned long coffset = 0;
static node * first = NULL;
static node ** last_p = &first;
static node * first_multilink = NULL;
static node ** last_multilink_p = &first_multilink;
static int fd = -1;

#define VERB_NONE	0
#define VERB_MINIMUM	1
#define VERB_SUB	2
#define VERB_MAX	3
#define VERB_EXCESSIVE	4

// Use gcc format argument checking on this function, which cannot return
static void fatal_error( int exitcode, const char *fmt, ... ) \
	__attribute__ (( noreturn,format (printf, 2, 3) ));

// Use gcc format argument checking on this function
static void verb_printf( int level, const char *fmt, ... ) \
	__attribute__ ((format (printf, 2, 3) ));

static void fatal_error( int exitcode, const char *fmt, ... ) {
    va_list v;

    va_start( v, fmt );
    vfprintf( stderr, fmt, v );

    exit(exitcode);
}

static void verb_printf( int level, const char *fmt, ... ){
    if ( level <= verbose ) {
	va_list v;
	va_start( v,fmt );
	vprintf(fmt, v);
    }
}

static void *mymalloc( size_t size ) {
    void *p = malloc(size);
    if ( !p ) {
	fatal_error( EXIT_MALLOC, "Out of memory allocating %d bytes\n", size );
    }
    return p;
}

static void myrealloc( void **o, size_t newsize ) {
    if ( *o == NULL )
	*o = mymalloc( newsize );
    else if ( !(*o = realloc( *o, newsize )) ) {
	fatal_error( EXIT_MALLOC, "Out of memory re-allocating %d bytes\n", newsize );
    }
}

static void outputlong( unsigned char *b, unsigned long w ) {
    if ( bigendian ) {
	b[0] = (w>>24) & 0xff;
	b[1] = (w>>16) & 0xff;
	b[2] = (w>> 8) & 0xff;
	b[3] = (w    ) & 0xff;
    } else {
	b[3] = (w>>24) & 0xff;
	b[2] = (w>>16) & 0xff;
	b[1] = (w>> 8) & 0xff;
	b[0] = (w    ) & 0xff;
    }
}

static void outputshort( unsigned char *b, unsigned short w ) {
    if ( bigendian ) {
	b[0] = (w>> 8) & 0xff;
	b[1] = (w    ) & 0xff;
    } else {
	b[1] = (w>> 8) & 0xff;
	b[0] = (w    ) & 0xff;
    }
}

static unsigned long ConvertMode( unsigned long posix_mode ) {
    unsigned long result = 0;
    if ( S_ISDIR( posix_mode ) ) result |= 1<<0;
    if ( S_ISCHR( posix_mode ) ) result |= 1<<1;
    if ( S_ISBLK( posix_mode ) ) result |= 1<<2;
    if ( S_ISREG( posix_mode ) ) result |= 1<<3;
    if ( S_ISFIFO(posix_mode ) ) result |= 1<<4;
    // We cannot create MQ, SEM, or SHM entries here
    if ( posix_mode & S_IRUSR )  result |= 1<<16;
    if ( posix_mode & S_IWUSR )  result |= 1<<17;
    if ( posix_mode & S_IXUSR )  result |= 1<<18;
    if ( posix_mode & S_IRGRP )  result |= 1<<19;
    if ( posix_mode & S_IWGRP )  result |= 1<<20;
    if ( posix_mode & S_IXGRP )  result |= 1<<21;
    if ( posix_mode & S_IROTH )  result |= 1<<22;
    if ( posix_mode & S_IWOTH )  result |= 1<<23;
    if ( posix_mode & S_IXOTH )  result |= 1<<24;
    if ( posix_mode & S_ISUID )  result |= 1<<25;
    if ( posix_mode & S_ISGID )  result |= 1<<26;
    return result;
}

static const char *AddDirEntry( const char *name, node *parent_node, int node_num ) {
    int this_size = ((strlen(name) + 4 + 4 + 1) + 31) & ~31;
    int start = parent_node->size;
    romfs_dirent *g;
    myrealloc( (void**)&parent_node->entry, (parent_node->size += this_size) );
    g = (romfs_dirent *)((unsigned char *)parent_node->entry + start);
    memset( (void*)g, '\0', this_size );
    outputlong( (char*)&g->node, node_num);
    outputlong( (char*)&g->next, parent_node->size);
    strcpy(g->name,name);
    verb_printf( VERB_MAX, "\t%s --> node %d\n", name, node_num );
    return (const char *)g->name;
}

extern int errno;

static node * FindLink( dev_t d, ino_t i ) {
    // See if the node has been previously included by checking the device/inode
    // combinations of all known multi-linked nodes
    node *np = first_multilink;

    for ( ; np ; np = np->next_multilink ) {
	if ( np->device == d && np->inode == i )
	    return np;
    }
    return NULL;
}

static node * GetNodeInfo( const char *path, const char *name, int *hlink ) {
    char newpath[1024];
    node *node, *lnode;
    struct stat stbuff;

    sprintf(newpath,"%s/%s",path,name);
    if ( (get_status(newpath,&stbuff)) < 0 ) {
	fatal_error(EXIT_FILESYS, "stat(%s) failed: %s\n", newpath, strerror(errno));
    }
    if ( !(stbuff.st_mode & S_IRUSR) ) {
	fatal_error(EXIT_FILESYS, "\"%s\" is not readable\n", newpath );
    }
    if ( hardlinks && S_ISREG( stbuff.st_mode ) && stbuff.st_nlink > 1 ) {

	// See if this node has already been loaded
	lnode = FindLink( stbuff.st_dev, stbuff.st_ino );

	if ( lnode ) {
	    lnode->nlink++;
	    *hlink = 1;
	    return lnode; // Return the found link instead
	}

	// Create a new node
	node = mymalloc( sizeof(struct node) );

	// Incorporate the new link into the 'multi-linked' node list
	*last_multilink_p = node;
	last_multilink_p = &node->next_multilink;
    } else {
	// Create a new node
	node = mymalloc( sizeof(struct node) );
    }
    node->path = strdup( newpath );
    // We re-calculate the size for directories
    node->size = IS_DIRECTORY( stbuff.st_mode ) ? 0 : stbuff.st_size;
    node->st_mode = stbuff.st_mode;
    node->uid = stbuff.st_uid;
    node->gid = stbuff.st_gid;
    node->ctime = stbuff.st_ctime;
    node->nodenum = nodes++;
    node->device = stbuff.st_dev;
    node->inode = stbuff.st_ino;
    // We always re-calculate the number of links
    node->nlink = IS_DIRECTORY( stbuff.st_mode ) ? 2 : 1;
    node->entry = NULL;
    node->entry_size = 0;
    node->offset = 0;
    node->sibling = NULL;
    node->child = NULL;
    node->next_in_rom = NULL;
    node->next_multilink = NULL;
    *hlink = 0;
    return node;
}

static void ScanDirectory(node *mynode, int p_node) {

    DIR *dh;
    struct dirent *e;
    node **last_p = &mynode->child;
    node *th;
    int was_hardlinked;

    if ( (dh  = opendir( mynode->path )) == NULL ) {
	perror(mynode->path);
	return;
    }

    verb_printf(VERB_EXCESSIVE, "Construct directory '%s'(%d):\n", 
	    mynode->path, mynode->nodenum );

    // Add . & .. here because they MUST be present in the image
    AddDirEntry( ".", mynode, mynode->nodenum );
    AddDirEntry( "..", mynode, p_node );

    while ( (e = readdir( dh )) ) {
	// Ignore . & .. here because they MAY NOT be in the host filesystem
	if ( strcmp(e->d_name,".") && strcmp(e->d_name,"..") ) {
	    

	    th = GetNodeInfo( mynode->path, e->d_name, &was_hardlinked );
	    AddDirEntry( e->d_name, mynode, th->nodenum );

	    if ( !was_hardlinked ) {
		verb_printf( VERB_EXCESSIVE, "\t\tNew node %d for entry '%s'\n", th->nodenum, e->d_name);
		*last_p = th;
		last_p = &th->sibling;
	    } else {
		verb_printf( VERB_EXCESSIVE, "\t\tRe-used node %d for entry '%s'\n", th->nodenum, e->d_name);
	    }
	}
    }
    closedir( dh );
    verb_printf(VERB_EXCESSIVE,"Completed '%s'. Checking for child directories...\n", mynode->path);

    for ( th = mynode->child ; th ; th = th->sibling ) {
	if ( IS_DIRECTORY( th->st_mode ) ) {
	    mynode->nlink++;
	    ScanDirectory( th, mynode->nodenum );
	}
    }
}

static void AllocateSpaceToDirectories( node *first ) {
    node *np;

    for ( np = first ; np ; np = np->sibling ) {
	if ( IS_DIRECTORY( np->st_mode ) ) {
	    // The first node is a directory. Add its data
	    np->offset = coffset;
	    np->entry_size = ALIGN_TO( np->size, DIRECTORY_ALIGN );
	    coffset += np->entry_size;

	    verb_printf( VERB_MAX, "\t\tnode %5d : 0x%06lX (+0x%05X)\n", 
			np->nodenum, np->offset, np->entry_size );

	    // Link this node into the write order chain.
	    // For node 0 (the root), this will overwrite the first pointer with itself
	    *last_p = np;
	    last_p = &np->next_in_rom;
	}
    }
    
    // Now add any child directories
    for ( np = first ; np ; np = np->sibling ) {
	if ( IS_DIRECTORY( np->st_mode ) && np->child )
	    AllocateSpaceToDirectories( np->child );
    }
}

static void AllocateSpaceToDataFiles( node *first ) {
    node *np;

    // There are two loops below. It CAN be done in just one, but this re-orders
    // the file positions in relation to their inode numbers. To keep it simple
    // to check, allocation takes place in the first loop, recursion in the second

    // Search for child data files
    for ( np = first->child ; np ; np = np->sibling ) {
	if ( IS_DATAFILE( np->st_mode ) || IS_SYMLINK( np->st_mode ) ) {
	    np->offset = coffset;
	    np->entry_size = ALIGN_TO( np->size, DATA_ALIGN );
	    coffset += np->entry_size;

	    // Link in to the rom write order list
	    *last_p = np;
	    last_p = &np->next_in_rom;

	    verb_printf( VERB_MAX, "\t\tnode %5d : 0x%06lX (+0x%05X)\n", 
			np->nodenum, np->offset, np->entry_size );
	}
    }

    // Recurse into sub-directories
    for ( np = first->child ; np ; np = np->sibling ) {
	if ( IS_DIRECTORY( np->st_mode ) ) {
	    AllocateSpaceToDataFiles( np );
	}
    }
}

static void AllocateSpaceToExecutables( node *first ) {
    node *np;

    // The first node is a directory. Don't bother with that...

    // Search for child executables
    for ( np = first->child ; np ; np = np->sibling ) {
	if ( IS_EXECUTABLE( np->st_mode ) ) {
	    np->offset = coffset;
	    np->entry_size = ALIGN_TO( np->size, EXEC_ALIGN );
	    coffset += np->entry_size;

	    // Link in to the rom write order list
	    *last_p = np;
	    last_p = &np->next_in_rom;

	    verb_printf( VERB_MAX, "\t\tnode %5d : 0x%06lX (+0x%05X)\n", 
			np->nodenum, np->offset, np->entry_size );
	}
    }

    // Recurse into sub-directories
    for ( np = first->child ; np ; np = np->sibling ) {
	if ( IS_DIRECTORY( np->st_mode ) ) {
	    AllocateSpaceToExecutables( np );
	}
    }
}

static void WriteNode( int fd, node *np ) {
    romfs_node anode;
    char padhere[9];
    outputlong( (char*) &anode.mode, ConvertMode( np->st_mode ) );
    outputlong( (char*) &anode.nlink, np->nlink );
    outputshort((char*) &anode.uid, np->uid );
    outputshort((char*) &anode.gid, np->gid );
    outputlong( (char*) &anode.size, np->size );
    outputlong( (char*) &anode.ctime, np->ctime );
    outputlong( (char*) &anode.data_offset, np->offset );
    sprintf( padhere, "<%6d>", np->nodenum );
    memcpy( anode.pad, padhere, 8 );
    if ( dowrite && write( fd, (void*)&anode, sizeof(anode) ) != sizeof(anode) )
	fatal_error(EXIT_WRITE, "Error writing node %d (%s): %s\n", np->nodenum, np->path, strerror(errno) );
}

static int WriteNodeAndSiblings( int fd, int nodenum, node *first ) {
    node *np;

    for ( np = first ; np ; np = np->sibling ) {
	if ( np->nodenum != nodenum++ ) {
	    fatal_error(EXIT_BUG, "BUG: Out of sequence node number; got %d, expected %d\n", np->nodenum, nodenum-1);
	}
	WriteNode( fd, np );
    }

    for ( np = first ; np ; np = np->sibling ) {
	if ( IS_DIRECTORY( np->st_mode ) && np->child ) {
	    nodenum = WriteNodeAndSiblings( fd, nodenum, np->child );
	}
    }
    return nodenum;
}

static void WriteNodeTable( int fd ) {
    romfs_disk header;
    int wnodes;

    outputlong( (char*) &header.magic, ROMFS_MAGIC );
    outputlong( (char*) &header.nodecount, nodes );
    outputlong( (char*) &header.disksize, coffset );
    outputlong( (char*) &header.dev_id, 0x01020304 );
    strcpy( header.name, "ROMFS v1.0" );
    if ( dowrite && write( fd, (void*)&header, sizeof(header) ) != sizeof(header) )
	fatal_error(EXIT_WRITE, "Error writing ROMFS header: %s\n", strerror(errno) );

    if ( (wnodes = WriteNodeAndSiblings( fd, 0, first )) != nodes ) {
	fatal_error(EXIT_BUG, "BUG: Lost/gained some nodes; wrote %d, expected %d\n", wnodes, nodes );
    }
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

static void WriteData( int fd, node *np ) {
    char newpath[1024];
    int ffd;
    unsigned long todo;

    if ( IS_SYMLINK( np->st_mode ) ) {
	if ( (ffd = readlink( np->path, newpath, sizeof(newpath) )) < 0 )
	    fatal_error(EXIT_FILESYS, "Error reading symlink \"%s\": %s\n", np->path, strerror(errno) );

	if ( !dowrite ) return;

	if ( lseek( fd, np->offset, SEEK_SET ) != np->offset )
	    fatal_error(EXIT_SEEK, "Error seeking to offset 0x%lX: %s\n", np->offset, strerror(errno) );

	if ( write( fd, newpath, ffd ) != ffd )
	    fatal_error(EXIT_WRITE, "Write error: %s\n", strerror(errno) );

	return;
    }
    
    if ( (ffd=open(np->path, O_RDONLY | O_BINARY )) < 0 )
	fatal_error(EXIT_FILESYS, "Error opening \"%s\": %s\n", np->path, strerror(errno) );

    if ( dowrite && lseek( fd, np->offset, SEEK_SET ) != np->offset )
	fatal_error(EXIT_SEEK, "Error seeking to offset 0x%lX: %s\n", np->offset, strerror(errno) );

    todo = np->size;
    while ( todo >= 1024 ) {
	if ( read( ffd, newpath, 1024 ) != 1024 )
	    fatal_error(EXIT_FILESYS, "Error reading file \"%s\" at offset 0x%lX: %s\n", np->path, np->size - todo, strerror(errno) );
	if ( dowrite && write( fd, newpath, 1024 ) != 1024 )
	    fatal_error(EXIT_WRITE, "Write error: %s\n", strerror(errno) );
	todo -= 1024;
    }

    if ( todo ) {
	if ( read( ffd, newpath, todo ) != todo )
	    fatal_error(EXIT_FILESYS, "Error reading file \"%s\" at offset 0x%lX: %s\n", np->path, np->size - todo, strerror(errno) );
	if ( dowrite && write( fd, newpath, todo ) != todo )
	    fatal_error(EXIT_WRITE, "Write error: %s\n", strerror(errno) );
    }

    close(ffd);

}

static void WriteDataBlocks( int fd, node *first ) {
    for ( ; first ; first = first->next_in_rom ) {
	if ( dowrite && lseek( fd, first->offset, SEEK_SET ) != first->offset )
	    fatal_error(EXIT_SEEK, "Error seeking to offset 0x%lX: %s\n", first->offset, strerror(errno) );
	if ( IS_DIRECTORY( first->st_mode ) ) {
	    if ( dowrite && write( fd, first->entry, first->size ) != first->size )
		fatal_error(EXIT_WRITE, "Write error: %s\n", strerror(errno) );
	} else {
	    WriteData( fd, first );
	}
    }
}

static void usage(void) {
    fprintf(stderr,"\n%s - Create an eCos ROMFS disk image from the files\n",prog);
    fprintf(stderr,"%*s   contained under a specified directory\n\n", strlen(prog), "");
    fprintf(stderr,"Usage: %s [options] <fs_root> <fs_file>\n", prog);
    fprintf(stderr," fs_root    is the directory containing the files to package into the ROMFS image\n");
    fprintf(stderr," fs_file    is the name of the ROMFS image file to create\n");
    fprintf(stderr,"          Options include:\n");
    fprintf(stderr," -v / -q    increase / decrease verbosity\n");
    fprintf(stderr," -n         do everything EXCEPT creating the output file\n");
    fprintf(stderr," -b         write a big-endian image (default is little endian)\n");
    fprintf(stderr," -l         collapse hard links to a single node\n");
    fprintf(stderr,"\n");
    exit(EXIT_ARGS);
}

int main(int ac, char *av[]) {
    int dummy;

    prog = av[0];

    // Check structure sizes
    if (sizeof(romfs_node) != 32) {
	fatal_error(EXIT_COMPILE , "Size of romfs_node is %d, NOT 32\n", sizeof(romfs_node) );
    } else if (sizeof(romfs_dirent) != 8) {
	fatal_error(EXIT_COMPILE , "Size of romfs_dirent is %d, NOT 8\n", sizeof(romfs_dirent) );
    } else if (sizeof(romfs_disk) != 32) {
	fatal_error(EXIT_COMPILE , "Size of romfs_disk is %d, NOT 32\n", sizeof(romfs_disk) );
    }

    // Parse option arguments
    while ( ac > 1 && av[1][0] == '-' ) {
	char *o = &av[1][1];
	for ( ; *o ; o++ ) {
	    switch ( *o ) {
		case 'q' :
		    verbose--;
		    break;
		case 'v' :
		    verbose++;
		    break;
		case 'n' :
		    dowrite = 0;
		    break;
		case 'b' :
		    bigendian = 1;
		    break;
		case 'l' :
		    hardlinks = 1;
		    break;
		default :
		    fprintf(stderr,"%s: Invalid flag -%c\n", prog, *o );
		    usage();
	    }
	}
	av++; ac--;
    }

    // Check remaining arguments
    if ( ac != 3 ) usage();


    verb_printf( VERB_MINIMUM, "%s: Verbosity %d %s%s endian\n",
		prog, verbose,
		dowrite ? "" : "no write, ",
		bigendian ? "big" : "little" );

    // Phase 1. Recursively scan the root directory for files and directories.
    verb_printf(VERB_MINIMUM, "Phase 1  - Build file list\n");

    first = GetNodeInfo( av[1], ".", &dummy );	// Initialize the root node entry.
    ScanDirectory( first, 0 );

    // Phase 2. Work out space allocations for filesystem
    verb_printf(VERB_MINIMUM, "Phase 2  - Calculate space allocation\n");
    coffset = sizeof(romfs_disk) + nodes * sizeof(romfs_node);
    verb_printf(VERB_MAX,"\t\tnode table : 0x000000 (+0x%05lX) %d nodes\n", coffset, nodes );
    
    // Phase 2a. Work out space allocations for the directories of the filesystem
    verb_printf(VERB_SUB,"Phase 2a -     * Directories\n");
    coffset = ALIGN_TO( coffset, DIRECTORY_ALIGN );
    AllocateSpaceToDirectories( first );

    // Phase 2b. Work out space allocations for the data files of the filesystem
    verb_printf(VERB_SUB,"Phase 2b -     * Regular files\n");
    coffset = ALIGN_TO( coffset, DATA_ALIGN );
    AllocateSpaceToDataFiles( first );

    // Phase 2c. Work out space allocations for the executable files of the filesystem
    verb_printf(VERB_SUB,"Phase 2c -     * Executable files\n");
    coffset = ALIGN_TO( coffset, EXEC_ALIGN );
    AllocateSpaceToExecutables( first );

    // Round off the image size...
    coffset = ALIGN_TO( coffset, EXEC_ALIGN );

    // Phase 3. Write out the image file
    verb_printf(VERB_MINIMUM, "Phase 3  - Construct ROMFS image file (%ld kb)\n", ALIGN_TO( coffset, 1024 )/1024);

    if ( dowrite ) {
	if ( (fd = open( av[2], O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666 )) < 0 ) {
	    fatal_error(EXIT_WRITE,"Failed to open output file '%s', errno=%d\n", av[2], errno );
	}
    } else {
	verb_printf(VERB_NONE,"           (No image is being written)\n");
    }

    verb_printf(VERB_SUB,"Phase 3a -     * Node table\n");
    WriteNodeTable( fd );

    verb_printf(VERB_SUB,"Phase 3b -     * Data blocks\n");
    WriteDataBlocks( fd, first );

    if ( fd >= 0 ) close(fd);

    verb_printf(VERB_MINIMUM,  "%s completed\n", av[2] );

    return 0;
}
