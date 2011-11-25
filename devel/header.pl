#!/usr/bin/perl
use strict;
# Core Modules
use Cwd qw(getcwd abs_path);
use File::Find;
use File::Basename;
use Data::Dumper;
# Modules
use Template;

my $FETCH = {
   c => [
      ['const\s+char\s+\*progdesc\s+=\s+"(.*?)"', 'DESC']
   ]
};
my $TEMPLATES = {
   h => [
      ['\/\*.*?\* Monitoring Plugin.*?\*\/\n', "header_h.tmpl"]
   ],
   c => [
      ['\/\*.*?\* Monitoring Plugin.*?\*\/\n', "header_c.tmpl"],
      ['\n*(\/\*\s+vim.*?\*\/)?\n$', "vimbang_c.tmpl"]
   ],
   php => [
      ['^<\?php.*?#\n\n', "header_php.tmpl"],
      ['\n+(#\s+vim.*?\n)?\n\?>\n$', "footer_php.tmpl"]
   ]
};
my $YEAR = (localtime)[5] + 1900;

my $tt = Template->new({
      INCLUDE_PATH => abs_path(dirname($0)),
      INTERPOLATE  => 0,
   }) || die "$Template::ERROR\n";

sub slurp {
   my $file = shift;
   my $src;
   open INFILE, $file or die "can't open $file $!";
   foreach my $line (<INFILE>) {
      $src .= $line
   }
   close (INFILE);
   return $src;
}

sub wanted {
   if($File::Find::name =~ /\/\./) {
      $File::Find::prune = 1;
      next;
   }
   my $FILE = $_;
   next if($FILE =~ /~$/);
   my $SHORTNAME = $FILE;

   my $subs = {
      FILENAME => $FILE,
      YEAR => $YEAR,
      PARENT => basename($File::Find::dir)
   };
   my $src;
   my $new = '';

   if ($FILE =~ /^(.*)\.(\w+)$/) {
      next if(!defined $TEMPLATES->{$2});

      $subs->{SHORTNAME} = $1;

      print "$File::Find::name\n";

      my $action;

      my $data = slurp($FILE);

      foreach $action (@{$FETCH->{$2}}) {
	 if ( $data =~ /$action->[0]/ ) {
	    $subs->{$action->[1]} = $1;
	 }
      }

      foreach $action (@{$TEMPLATES->{$2}}) {
	 print " $action->[1]\n";
	 $new = '';

	 my $ret = $tt->process($action->[1], $subs, \$new);

	 if ($ret == 1) {
	    $data =~ s/$action->[0]/$new/s;
	 } else {
	    print $tt->error."\n";
	    next;
	 }
      }

      open (MYFILE, ">$FILE");
      print MYFILE $data;
      close (MYFILE);
   }
}

find(\&wanted, getcwd);
