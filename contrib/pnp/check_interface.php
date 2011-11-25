<?php
#
# Monitoring Plugin PNP - check_interface.php
#
# Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# $Id$
#

$opt[1] = "--vertical-label \"bits per second\" --title \"Traffic $hostname / $servicedesc\" ";
$opt[2] = "--vertical-label \"errors per second\" --title \"Error $hostname / $servicedesc\" ";

$ds_name[1] = "Traffic";
$ds_name[2] = "Error";

$def[1]  = rrd::def("ifInOctets", $RRDFILE[1], $DS[1], "AVERAGE");
$def[1] .= rrd::def("ifOutOctets", $RRDFILE[3], $DS[3], "AVERAGE");
$def[1] .= rrd::def("ifSpeed", $RRDFILE[5], $DS[5], "AVERAGE");
$def[1] .= rrd::vdef('ifSpeedLast', 'ifSpeed,LAST' );

$def[2]  = rrd::def("ifInErrors", $RRDFILE[2], $DS[2], "AVERAGE");
$def[2] .= rrd::def("ifOutErrors", $RRDFILE[4], $DS[4], "AVERAGE");

$def[1] .= rrd::hrule("ifSpeedLast", "#000000", rrd::cut("Speed",10));
$def[1] .= rrd::gprint("ifSpeed", array("LAST"), "%6.2lf%sbit/s");
$def[1] .= rrd::area("ifInOctets", "#00FF00", rrd::cut("Inbound",10));
$def[1] .= rrd::gprint("ifInOctets", array("LAST","MAX","AVERAGE"), "%6.2lf%sbit/s");
$def[1] .= rrd::line1("ifOutOctets", "#0000FF", rrd::cut("Outbound",10));
$def[1] .= rrd::gprint("ifOutOctets", array("LAST","MAX","AVERAGE"), "%6.2lf%sbit/s");

$def[2] .= rrd::area("ifInErrors", "#800000", rrd::cut("Inbound",10));
$def[2] .= rrd::gprint("ifInErrors", array("LAST","MAX","AVERAGE"), "%4.2lf%s errors/s");
$def[2] .= rrd::area("ifOutErrors", "#FF0000", rrd::cut("Outbound",10), true);
$def[2] .= rrd::gprint("ifOutErrors", array("LAST","MAX","AVERAGE"), "%4.2lf%s errors/s");

# vim: set ts=4 sw=4 et syn=php :
?>
