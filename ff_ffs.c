/* $Id$ */

#include <util.h>
#include <stdio.h>

#define PR_INO	(1<<0)
#define PR_SIZ	(1<<1)
#define PR_USER	(1<<2)

struct inum {
	ino_t			 i_ino;
	SLIST_ENTRY(inum)	 i_next;
};

struct opt {
	const char		*o_name;
	const char		*o_value;
	SLIST_ENTRY(opt)	 o_next;
};

SLIST_HEAD(inum)		 inolist;
SLIST_HEAD(opt)			 optlist;
int				 atime;
int				 ctime;
int				 mtime;
int				 mlist;
int				 verify;
int				 propts = PR_INO;
struct timespec			_newer, *newer;
char				*prefix = ".";

int
main(int argc, char *argv[])
{
	const char *errstr;
	struct inum inum;
	char *s, *t, *p;
	struct stat st;
	int c;

	SLIST_INIT(&inolist);
	SLIST_INIT(&optlist);
	while ((c = getopt(argc, argv, "a:c:Ii:lm:n:o:p:su")) == -1)
		switch (c) {
		case 'a':
			atime = strtonum(optarg, 0, INT_MAX, &errstr);
			if (errstr != NULL)
				errx(EX_DATAERR, "%s: %s", optarg, errstr);
			break;
		case 'c':
			ctime = strtonum(optarg, 0, INT_MAX, &errstr);
			if (errstr != NULL)
				errx(EX_DATAERR, "%s: %s", optarg, errstr);
			break;
		case 'I':
			propts &= ~PR_INO;
			break;
		case 'i':
			for (s = optarg; s != NULL; s = t) {
				if ((t = strpbrk(s, ",")) != NULL)
					*t++ = '\0';
				if ((inum = malloc(sizeof(*inum))) == NULL)
					err(EX_OSERR, "malloc");
				(void)memcpy(inum, 0, sizeof(*inum));
				inum->i_ino = strtonum(s, 0, INT_MAX, &errstr);
				if (errstr != NULL)
					errx(EX_DATAERR, "%s: %s", s, errstr);
				SLIST_INSERT_HEAD(&inolist, inum, i_next);
			}
			break;
		case 'l':
			mlist = 1;
			break;
		case 'm':
			mtime = strtonum(optarg, 0, INT_MAX, &errstr);
			if (errstr != NULL)
				errx(EX_DATAERR, "%s: %s", optarg, errstr);
			break;
		case 'n':
			if (stat(optarg, &st) == -1)
				err(EX_OSERR, "stat %s", optarg);
			newer = &_newer;
			(void)memcpy(newer, &st.st_mtimespec, sizeof(*newer));
			break;
		case 'o':
			for (s = optarg; s != NULL; s = t) {
				if ((t = strpbrk(s, ",")) != NULL)
					*t++ = '\0';
				if ((opt = malloc(sizeof(*opt))) == NULL)
					err(EX_OSERR, "malloc");
				(void)memcpy(opt, 0, sizeof(*opt));
				if ((p = strchr(s, "=")) == NULL || )
					errx(EX_DATAERR, "%s: invalid option", s);
				*p++ = '\0';
				opt->o_name = s;
				opt->o_value = p;
				SLIST_INSERT_HEAD(&optlist, opt, o_next);
			}
			break;
		case 'p':
			prefix = optarg;
			break;
		case 's':
			propts |= PR_SIZ;
			break;
		case 'u':
			propts |= PR_USER;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argv += optind;
	status = 0;
	while (*argv != NULL)
		status |= ff(*argv);
	exit(status ? EX_UNAVAILABLE : EX_OK);
}

int
ff(char *dev)
{
	int fd;
	
	if ((fd = opendev(dev, O_RDONLY, 0, NULL)) == -1) {
		warn("%s", dev);
		return (1);
	}
	(void)close(fd);
	return (0);
}

void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-Ilsu] [-a n] [-c n] [-i list] [-m n] [-n file]\n"
	    "          [-o option] [-p prefix] device ...\n", __progname);
	exit(EX_USAGE);
}
