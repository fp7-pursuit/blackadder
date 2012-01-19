#-
# Copyright (C) 2011  Oy L M Ericsson Ab, NomadicLab
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Alternatively, this software may be distributed under the terms of the
# BSD license.
#
# See LICENSE and COPYING for more details.
#

CC?=gcc

NAME1=test_pub
NAME2=test_nb_sub

.PHONY: all clean

all: $(NAME1) $(NAME2) 

$(NAME1): $(NAME1).c
	$(CC) -Wall $(CFLAGS) -o $(NAME1) $(NAME1).c $(LDFLAGS) -lblackadder_c -lpthread

$(NAME2): $(NAME2).c
	$(CC) -Wall $(CFLAGS) -o $(NAME2) $(NAME2).c $(LDFLAGS) -lblackadder_c -lpthread

clean:
	rm -f $(NAME1) $(NAME2)
