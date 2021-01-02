# Copyright (c) 2003-2020 Christian Jaeger, copying@christianjaeger.ch
#
# This is free software, offered under either the same terms as perl 5
# or the terms of the Artistic License version 2 or the terms of the
# MIT License (Expat version). See the file COPYING.md that came
# bundled with this file.

package PFLANZE::cbuild;

use strict; use warnings FATAL => 'uninitialized';
use experimental 'signatures';

use Exporter qw(import);

our @EXPORT_OK= qw(
    singlequote_sh
    possibly_singlequote_sh
    xxsystem_safe
    xgetfile_utf8
    xexec
    min
    max
    tempdir
    );

# (Plus accessing Chj::xperlfunc::* directly.)


# Slightly adapted copies from the FunctionalPerl package to avoid the
# dependency and slow load times:

use POSIX qw(_exit EEXIST);

# use lib "/opt/functional-perl/lib";
# use Chj::singlequote qw(possibly_singlequote_sh);

sub singlequote_sh($str, $alternative = undef) {
    if (defined $str) {
        $str =~ s/\'/'\\\''/sg;
        "'$str'"
    } else {
        defined($alternative) ? $alternative : "undef"
    }
}

sub possibly_singlequote_sh($str) {
    if ($str =~ m{^[=\w/.-]+\z}) {
        $str
    } else {
        singlequote_sh $str
    }
}

# use Chj::xperlfunc qw(xsystem_safe xgetfile_utf8);

sub xxsystem_safe {
    @_ > 0 or die "xxsystem_safe: missing arguments";
    no warnings;
    (system { $_[0] } @_) >= 0
        or die "xxsystem_safe: could not start command '$_[0]': $!";
    $? == 0 or die "xxsystem_safe: process terminated with status $?";
}

sub xgetfile_utf8($path) {
    open my $in, "<", $path or die "xgetfile_utf8($path): open: $!";
    binmode $in, ":encoding(UTF-8)" or die "binmode";
    local $/;
    my $cnt = <$in> // die "xgetfile_utf8($path): read: $!";
    close $in or die "xgetfile_utf8($path): close: $!";
    $cnt
}

sub xexec {
    exec @_ or _exit 127; #?
}


# use FP::Div qw(min max);

sub min {
    my $x = shift;
    for (@_) {
        $x = $_ if $_ < $x
    }
    $x
}

sub max {
    my $x = shift;
    for (@_) {
        $x = $_ if $_ > $x
    }
    $x
}


package Chj::xperlfunc::xstat {

    sub dev     { shift->[0] }
    sub ino     { shift->[1] }
    sub mode    { shift->[2] }
    sub nlink   { shift->[3] }
    sub uid     { shift->[4] }
    sub gid     { shift->[5] }
    sub rdev    { shift->[6] }
    sub size    { shift->[7] }
    sub atime   { shift->[8] }
    sub mtime   { shift->[9] }
    sub ctime   { shift->[10] }
    sub blksize { shift->[11] }
    sub blocks  { shift->[12] }

    sub permissions     { shift->[2] & 07777 }
    sub permissions_oct { sprintf('%o', shift->permissions) }
    sub permissions_u   { (shift->[2] & 00700) >> 6 }
    sub permissions_g   { (shift->[2] & 00070) >> 3 }
    sub permissions_o   { shift->[2] & 00007 }
    sub permissions_s   { (shift->[2] & 07000) >> 9 }
    sub setuid          { !!(shift->[2] & 04000) }

    sub setgid   { !!(shift->[2] & 02000) }
    sub sticky   { !!(shift->[2] & 01000) }
    sub filetype { (shift->[2] & 0170000) >> 12 }    # 4*3bits

}

package Chj::xperlfunc {
    use POSIX qw(ENOENT);
    use Carp qw(croak);

    our $time_hires = 0;

    sub stat_possiblyhires {
        if ($time_hires) {
            require Time::HiRes;    # (that's not slow, right?)
            if (@_) {
                @_ == 1 or croak "need 1 argument";
                Time::HiRes::stat($_[0])
            } else {
                Time::HiRes::stat($_)
            }
        } else {
            if (@_) {
                @_ == 1 or croak "need 1 argument";
                stat($_[0])
            } else {
                stat($_)
            }
        }
    }

    sub xstat {
        my @r;
        @_ <= 1 or croak "xstat: too many arguments";
        @r = stat_possiblyhires(@_ ? @_ : $_);
        @r or croak(@_ ? "xstat: '@_': $!" : "xstat: '$_': $!");
        my $self = \@r;
        bless $self, 'Chj::xperlfunc::xstat'
    }

    sub Xstat {
        my @r;
        @_ <= 1 or croak "Xstat: too many arguments";
        @r = stat_possiblyhires(@_ ? @_ : $_);
        @r or do {
            if ($! == ENOENT) {
                return;
            } else {
                croak(@_ ? "Xstat: '@_': $!" : "Xstat: '$_': $!");
            }
        };
        bless \@r, 'Chj::xperlfunc::xstat'
    }
}


# Written from scratch here:

sub tempdir () {
    my $tries= 0;
  TRY: {
      my $str = $$ . "-" . rand;
      $str=~ s/0?\.//;
      my $path= "/tmp/$str";
      if (mkdir $path) {
          $path
      } elsif ($! == EEXIST and $tries++ < 5) {
          redo TRY;
      } else {
          die "mkdir($path): $!";
      }
    }
}

1
