# Makefile for memtier_benchmark
# Copyright (C) 2011-2017 Redis Labs Ltd.

# This file is part of memtier_benchmark.

# memtier_benchmark is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects

EXTRA_DIST = README.md COPYING

bin_PROGRAMS = memtier_benchmark
completionsdir=$(BASH_COMPLETION_DIR)
completions_DATA=bash-completion/memtier_benchmark

memtier_benchmark_CPPFLAGS = \
	$(LIBEVENT_CFLAGS) \
	$(LIBEVENT_OPENSSL_CFLAGS) \
	$(LIBCRYPTO_CFLAGS) \
	$(LIBSSL_CFLAGS)
memtier_benchmark_SOURCES = \
	memtier_benchmark.cpp memtier_benchmark.h \
	client.cpp client.h \
	mongoose.c mongoose.h \
	cluster_client.cpp cluster_client.h \
	shard_connection.cpp shard_connection.h connections_manager.h \
	run_stats_types.cpp run_stats_types.h \
	run_stats.cpp run_stats.h \
	JSON_handler.cpp JSON_handler.h \
	protocol.cpp protocol.h \
	obj_gen.cpp obj_gen.h \
	item.cpp item.h \
	file_io.cpp file_io.h \
	config_types.cpp config_types.h \
	deps/hdr_histogram/hdr_histogram_log.c deps/hdr_histogram/hdr_histogram_log.h deps/hdr_histogram/byteorder.h \
	deps/hdr_histogram/hdr_histogram.c deps/hdr_histogram/hdr_histogram.h \
	deps/hdr_histogram/hdr_time.c deps/hdr_histogram/hdr_time.h deps/hdr_histogram/hdr_encoding.c deps/hdr_histogram/hdr_encoding.h

memtier_benchmark_LDADD = \
	$(LIBEVENT_LIBS) \
	$(LIBEVENT_OPENSSL_LIBS) \
	$(LIBCRYPTO_LIBS) \
	$(LIBSSL_LIBS)

dist_man1_MANS = memtier_benchmark.1

.PHONY: rebuild-man
rebuild-man:
	help2man --name="NoSQL benchmark tool" --no-info --no-discard-stderr --output=memtier_benchmark.1 $(builddir)/memtier_benchmark
