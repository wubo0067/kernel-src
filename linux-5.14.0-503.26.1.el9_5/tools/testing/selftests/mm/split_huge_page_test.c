// SPDX-License-Identifier: GPL-2.0
/*
 * A test of splitting PMD THPs and PTE-mapped THPs from a specified virtual
 * address range in a process via <debugfs>/split_huge_pages interface.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <malloc.h>
#include <stdbool.h>
#include "vm_util.h"
#include "../kselftest.h"

uint64_t pagesize;
unsigned int pageshift;
uint64_t pmd_pagesize;

#define SPLIT_DEBUGFS "/sys/kernel/debug/split_huge_pages"
#define INPUT_MAX 80

#define PID_FMT "%d,0x%lx,0x%lx"
#define PATH_FMT "%s,0x%lx,0x%lx"

#define PFN_MASK     ((1UL<<55)-1)
#define KPF_THP      (1UL<<22)

int is_backed_by_thp(char *vaddr, int pagemap_file, int kpageflags_file)
{
	uint64_t paddr;
	uint64_t page_flags;

	if (pagemap_file) {
		pread(pagemap_file, &paddr, sizeof(paddr),
			((long)vaddr >> pageshift) * sizeof(paddr));

		if (kpageflags_file) {
			pread(kpageflags_file, &page_flags, sizeof(page_flags),
				(paddr & PFN_MASK) * sizeof(page_flags));

			return !!(page_flags & KPF_THP);
		}
	}
	return 0;
}

static void write_file(const char *path, const char *buf, size_t buflen)
{
	int fd;
	ssize_t numwritten;

	fd = open(path, O_WRONLY);
	if (fd == -1)
		ksft_exit_fail_msg("%s open failed: %s\n", path, strerror(errno));

	numwritten = write(fd, buf, buflen - 1);
	close(fd);
	if (numwritten < 1)
		ksft_exit_fail_msg("Write failed\n");
}

static void write_debugfs(const char *fmt, ...)
{
	char input[INPUT_MAX];
	int ret;
	va_list argp;

	va_start(argp, fmt);
	ret = vsnprintf(input, INPUT_MAX, fmt, argp);
	va_end(argp);

	if (ret >= INPUT_MAX)
		ksft_exit_fail_msg("%s: Debugfs input is too long\n", __func__);

	write_file(SPLIT_DEBUGFS, input, ret + 1);
}

void split_pmd_thp(void)
{
	char *one_page;
	size_t len = 4 * pmd_pagesize;
	size_t i;

	one_page = memalign(pmd_pagesize, len);
	if (!one_page)
		ksft_exit_fail_msg("Fail to allocate memory: %s\n", strerror(errno));

	madvise(one_page, len, MADV_HUGEPAGE);

	for (i = 0; i < len; i++)
		one_page[i] = (char)i;

	if (!check_huge_anon(one_page, 4, pmd_pagesize))
		ksft_exit_fail_msg("No THP is allocated\n");

	/* split all THPs */
	write_debugfs(PID_FMT, getpid(), (uint64_t)one_page,
		(uint64_t)one_page + len);

	for (i = 0; i < len; i++)
		if (one_page[i] != (char)i)
			ksft_exit_fail_msg("%ld byte corrupted\n", i);


	if (!check_huge_anon(one_page, 0, pmd_pagesize))
		ksft_exit_fail_msg("Still AnonHugePages not split\n");

	ksft_test_result_pass("Split huge pages successful\n");
	free(one_page);
}

void split_pte_mapped_thp(void)
{
	char *one_page, *pte_mapped, *pte_mapped2;
	size_t len = 4 * pmd_pagesize;
	uint64_t thp_size;
	size_t i;
	const char *pagemap_template = "/proc/%d/pagemap";
	const char *kpageflags_proc = "/proc/kpageflags";
	char pagemap_proc[255];
	int pagemap_fd;
	int kpageflags_fd;

	if (snprintf(pagemap_proc, 255, pagemap_template, getpid()) < 0)
		ksft_exit_fail_msg("get pagemap proc error: %s\n", strerror(errno));

	pagemap_fd = open(pagemap_proc, O_RDONLY);
	if (pagemap_fd == -1)
		ksft_exit_fail_msg("read pagemap: %s\n", strerror(errno));

	kpageflags_fd = open(kpageflags_proc, O_RDONLY);
	if (kpageflags_fd == -1)
		ksft_exit_fail_msg("read kpageflags: %s\n", strerror(errno));

	one_page = mmap((void *)(1UL << 30), len, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (one_page == MAP_FAILED)
		ksft_exit_fail_msg("Fail to allocate memory: %s\n", strerror(errno));

	madvise(one_page, len, MADV_HUGEPAGE);

	for (i = 0; i < len; i++)
		one_page[i] = (char)i;

	if (!check_huge_anon(one_page, 4, pmd_pagesize))
		ksft_exit_fail_msg("No THP is allocated\n");

	/* remap the first pagesize of first THP */
	pte_mapped = mremap(one_page, pagesize, pagesize, MREMAP_MAYMOVE);

	/* remap the Nth pagesize of Nth THP */
	for (i = 1; i < 4; i++) {
		pte_mapped2 = mremap(one_page + pmd_pagesize * i + pagesize * i,
				     pagesize, pagesize,
				     MREMAP_MAYMOVE|MREMAP_FIXED,
				     pte_mapped + pagesize * i);
		if (pte_mapped2 == MAP_FAILED)
			ksft_exit_fail_msg("mremap failed: %s\n", strerror(errno));
	}

	/* smap does not show THPs after mremap, use kpageflags instead */
	thp_size = 0;
	for (i = 0; i < pagesize * 4; i++)
		if (i % pagesize == 0 &&
		    is_backed_by_thp(&pte_mapped[i], pagemap_fd, kpageflags_fd))
			thp_size++;

	if (thp_size != 4)
		ksft_exit_fail_msg("Some THPs are missing during mremap\n");

	/* split all remapped THPs */
	write_debugfs(PID_FMT, getpid(), (uint64_t)pte_mapped,
		      (uint64_t)pte_mapped + pagesize * 4);

	/* smap does not show THPs after mremap, use kpageflags instead */
	thp_size = 0;
	for (i = 0; i < pagesize * 4; i++) {
		if (pte_mapped[i] != (char)i)
			ksft_exit_fail_msg("%ld byte corrupted\n", i);

		if (i % pagesize == 0 &&
		    is_backed_by_thp(&pte_mapped[i], pagemap_fd, kpageflags_fd))
			thp_size++;
	}

	if (thp_size)
		ksft_exit_fail_msg("Still %ld THPs not split\n", thp_size);

	ksft_test_result_pass("Split PTE-mapped huge pages successful\n");
	munmap(one_page, len);
	close(pagemap_fd);
	close(kpageflags_fd);
}

void split_file_backed_thp(void)
{
	int status;
	int fd;
	ssize_t num_written;
	char tmpfs_template[] = "/tmp/thp_split_XXXXXX";
	const char *tmpfs_loc = mkdtemp(tmpfs_template);
	char testfile[INPUT_MAX];
	uint64_t pgoff_start = 0, pgoff_end = 1024;

	ksft_print_msg("Please enable pr_debug in split_huge_pages_in_file() for more info.\n");

	status = mount("tmpfs", tmpfs_loc, "tmpfs", 0, "huge=always,size=4m");

	if (status)
		ksft_exit_fail_msg("Unable to create a tmpfs for testing\n");

	status = snprintf(testfile, INPUT_MAX, "%s/thp_file", tmpfs_loc);
	if (status >= INPUT_MAX) {
		ksft_exit_fail_msg("Fail to create file-backed THP split testing file\n");
	}

	fd = open(testfile, O_CREAT|O_WRONLY, 0664);
	if (fd == -1) {
		ksft_perror("Cannot open testing file");
		goto cleanup;
	}

	/* write something to the file, so a file-backed THP can be allocated */
	num_written = write(fd, tmpfs_loc, strlen(tmpfs_loc) + 1);
	close(fd);

	if (num_written < 1) {
		ksft_perror("Fail to write data to testing file");
		goto cleanup;
	}

	/* split the file-backed THP */
	write_debugfs(PATH_FMT, testfile, pgoff_start, pgoff_end);

	status = unlink(testfile);
	if (status) {
		ksft_perror("Cannot remove testing file");
		goto cleanup;
	}

	status = umount(tmpfs_loc);
	if (status) {
		rmdir(tmpfs_loc);
		ksft_exit_fail_msg("Unable to umount %s\n", tmpfs_loc);
	}

	status = rmdir(tmpfs_loc);
	if (status)
		ksft_exit_fail_msg("cannot remove tmp dir: %s\n", strerror(errno));

	ksft_print_msg("Please check dmesg for more information\n");
	ksft_test_result_pass("File-backed THP split test done\n");
	return;

cleanup:
	umount(tmpfs_loc);
	rmdir(tmpfs_loc);
	ksft_exit_fail_msg("Error occurred\n");
}

int main(int argc, char **argv)
{
	ksft_print_header();

	if (geteuid() != 0) {
		ksft_print_msg("Please run the benchmark as root\n");
		ksft_finished();
	}

	ksft_set_plan(3);

	pagesize = getpagesize();
	pageshift = ffs(pagesize) - 1;
	pmd_pagesize = read_pmd_pagesize();
	if (!pmd_pagesize)
		ksft_exit_fail_msg("Reading PMD pagesize failed\n");

	split_pmd_thp();
	split_pte_mapped_thp();
	split_file_backed_thp();

	ksft_finished();
}
