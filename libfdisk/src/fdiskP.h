/*
 * fdiskP.h - private library header file
 *
 * Copyright (C) 2012 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#ifndef _LIBFDISK_PRIVATE_H
#define _LIBFDISK_PRIVATE_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "c.h"
#include "libfdisk.h"

/* features */
#define CONFIG_LIBFDISK_ASSERT
#define CONFIG_LIBFDISK_DEBUG

#ifdef CONFIG_LIBFDISK_ASSERT
#include <assert.h>
#endif

/*
 * Debug
 */
#if defined(TEST_PROGRAM) && !defined(LIBFDISK_DEBUG)
#define CONFIG_LIBFDISK_DEBUG
#endif

#ifdef CONFIG_LIBFDISK_DEBUG
# include <stdio.h>
# include <stdarg.h>

/* fdisk debugging flags/options */
#define FDISK_DEBUG_INIT	(1 << 1)
#define FDISK_DEBUG_CONTEXT	(1 << 2)
#define FDISK_DEBUG_TOPOLOGY    (1 << 3)
#define FDISK_DEBUG_GEOMETRY    (1 << 4)
#define FDISK_DEBUG_LABEL       (1 << 5)
#define FDISK_DEBUG_ALL		0xFFFF

# define ON_DBG(m, x)	do { \
				if ((FDISK_DEBUG_ ## m) & fdisk_debug_mask) { \
					x; \
				}	   \
			} while (0)

# define DBG(m, x)	do { \
				if ((FDISK_DEBUG_ ## m) & fdisk_debug_mask) { \
					fprintf(stderr, "%d: fdisk: %8s: ", getpid(), # m); \
					x;				\
				} \
			} while (0)

# define DBG_FLUSH	do { \
				if (fdisk_debug_mask && \
				    fdisk_debug_mask != FDISK_DEBUG_INIT) \
					fflush(stderr);			\
			} while(0)

static inline void __attribute__ ((__format__ (__printf__, 1, 2)))
dbgprint(const char *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

extern int fdisk_debug_mask;

#else /* !CONFIG_LIBFDISK_DEBUG */
# define ON_DBG(m,x) do { ; } while (0)
# define DBG(m,x) do { ; } while (0)
# define DBG_FLUSH do { ; } while(0)
#endif

typedef unsigned long long sector_t;

/*
 * Supported partition table types (labels)
 */
enum fdisk_labeltype {
	FDISK_DISKLABEL_DOS = 1,
	FDISK_DISKLABEL_SUN = 2,
	FDISK_DISKLABEL_SGI = 4,
	FDISK_DISKLABEL_AIX = 8,
	FDISK_DISKLABEL_OSF = 16,
	FDISK_DISKLABEL_MAC = 32,
	FDISK_DISKLABEL_GPT = 64,
	FDISK_DISKLABEL_ANY = -1
};

/*
 * Partition types
 */
struct fdisk_parttype {
	unsigned int	type;		/* type as number or zero */
	const char	*name;		/* description */
	char		*typestr;	/* type as string or NULL */

	unsigned int	flags;		/* FDISK_PARTTYPE_* flags */
};

enum {
	FDISK_PARTTYPE_UNKNOWN		= (1 << 1),
	FDISK_PARTTYPE_INVISIBLE	= (1 << 2),
	FDISK_PARTTYPE_ALLOCATED	= (1 << 3)
};

#define fdisk_parttype_is_unknown(_x)	((_x) && ((_x)->flags & FDISK_PARTTYPE_UNKNONW))
#define fdisk_parttype_is_invisible(_x)	((_x) && ((_x)->flags & FDISK_PARTTYPE_INVISIBLE))
#define fdisk_parttype_is_allocated(_x)	((_x) && ((_x)->flags & FDISK_PARTTYPE_ALLOCATED))

/*
 * Legacy CHS based geometry
 */
struct fdisk_geometry {
	unsigned int heads;
	sector_t sectors;
	sector_t cylinders;
};

struct fdisk_context {
	int dev_fd;         /* device descriptor */
	char *dev_path;     /* device path */
	unsigned char *firstsector; /* buffer with master boot record */

	/* topology */
	unsigned long io_size;		/* I/O size used by fdisk */
	unsigned long optimal_io_size;	/* optional I/O returned by device */
	unsigned long min_io_size;	/* minimal I/O size */
	unsigned long phy_sector_size;	/* physical size */
	unsigned long sector_size;	/* logical size */
	unsigned long alignment_offset;

	enum fdisk_labeltype disklabel;	/* current disklabel */

	/* alignment */
	unsigned long grain;		/* alignment unit */
	sector_t first_lba;		/* recommended begin of the first partition */

	/* geometry */
	sector_t total_sectors; /* in logical sectors */
	struct fdisk_geometry geom;

	/* label operations and description */
	const struct fdisk_label *label;
};

#define fdisk_is_disklabel(c, x) fdisk_dev_is_disklabel(c, FDISK_DISKLABEL_ ## x)

/*
 * Label specific operations
 */
struct fdisk_label {
	const char *name;

	/* array with partition types */
	struct fdisk_parttype	*parttypes;
	size_t			nparttypes;	/* number of items in parttypes[] */

	/* probe disk label */
	int (*probe)(struct fdisk_context *cxt);
	/* write in-memory changes to disk */
	int (*write)(struct fdisk_context *cxt);
	/* verify the partition table */
	int (*verify)(struct fdisk_context *cxt);
	/* create new disk label */
	int (*create)(struct fdisk_context *cxt);
	/* new partition */
	int (*part_add)(struct fdisk_context *cxt, int partnum, struct fdisk_parttype *t);
	/* delete partition */
	int (*part_delete)(struct fdisk_context *cxt, int partnum);
	/* get partition type */
	struct fdisk_parttype *(*part_get_type)(struct fdisk_context *cxt, int partnum);
	/* set partition type */
	int (*part_set_type)(struct fdisk_context *cxt, int partnum, struct fdisk_parttype *t);
	/* refresh alignment setting */
	int (*reset_alignment)(struct fdisk_context *cxt);
};

#endif /* _LIBFDISK_PRIVATE_H */
