<?php
#
# Monitoring Plugin PNP - check_mem.php
#
# Copyright (C) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# $Id$
#

$ds_name[1] = "Memory Usage";
$opt[1] = "-l0  --title \"Memory Usage for $hostname\" ";

$def[1]  = rrd::def("tot", $RRDFILE[1], $DS[1], "AVERAGE");
$def[1] .= rrd::def("slab", $RRDFILE[1], $DS[2], "AVERAGE");
$def[1] .= rrd::def("swapcached", $RRDFILE[1], $DS[3], "AVERAGE");
$def[1] .= rrd::def("pagetables", $RRDFILE[1], $DS[4], "AVERAGE");
$def[1] .= rrd::def("apps", $RRDFILE[1], $DS[5], "AVERAGE");
$def[1] .= rrd::def("memfree", $RRDFILE[1], $DS[6], "AVERAGE");
$def[1] .= rrd::def("buffers", $RRDFILE[1], $DS[7], "AVERAGE");
$def[1] .= rrd::def("cached", $RRDFILE[1], $DS[8], "AVERAGE");
$def[1] .= rrd::def("swap", $RRDFILE[1], $DS[9], "AVERAGE");

$def[1] .= rrd::line1("tot", "#000000", "Total memory");
$def[1] .= rrd::gprint("tot", array("LAST"), "%6.2lf%s");
$def[1] .= rrd::area("apps", "#FF0000", "Applications");
$def[1] .= rrd::gprint("apps", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[1] .= rrd::area("pagetables", "#0000FF", "Page Table  ", true);
$def[1] .= rrd::gprint("pagetables", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[1] .= rrd::area("slab", "#00FFFF", "Slab        ", true);
$def[1] .= rrd::gprint("slab", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[1] .= rrd::area("swapcached", "#FF8000", "Swap Cache  ", true);
$def[1] .= rrd::gprint("swapcached", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[1] .= rrd::area("cached", "#FFFF00", "Cache       ", true);
$def[1] .= rrd::gprint("cached", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[1] .= rrd::area("buffers", "#80FF00", "Buffer      ", true);
$def[1] .= rrd::gprint("buffers", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[1] .= rrd::area("memfree", "#00FF00", "Free        ", true);
$def[1] .= rrd::gprint("memfree", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[1] .= rrd::area("swap", "#800000", "Swap        ", true);
$def[1] .= rrd::gprint("swap", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");

# vim: set ts=4 sw=4 et syn=php :
?>
