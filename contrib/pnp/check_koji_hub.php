<?php
#
# Monitoring Plugin PNP - check_koji_hub.php
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
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# $Id$
#

$opt[1] = "--vertical-label \"$UNIT[1]\" --title \"Response Times $hostname / $servicedesc\" ";
$opt[2] = "--vertical-label \"Tasks\" --title \"Koji Tasks $hostname\" ";

$ds_name[1] = "Response Time";
$ds_name[2] = "Tasks";

$ds_name[1] = "Response Time";
$ds_name[2] = "Tasks";

$def[1] =  "DEF:var1=$RRDFILE[1]:$DS[1]:AVERAGE " ;
if ($WARN[1] != "") {
    $def[1] .= "HRULE:$WARN[1]#FFFF00 ";
}
if ($CRIT[1] != "") {
    $def[1] .= "HRULE:$CRIT[1]#FF0000 ";
}
$def[1] .= rrd::gradient("var1", "66CCFF", "0000ff", "$NAME[1]");
$def[1] .= rrd::line1("var1", "666666");
$def[1] .= rrd::gprint("var1", array("LAST","MAX","AVERAGE"), "%6.2lf $UNIT[1]");

$def[2]  = rrd::def("free", $RRDFILE[1], $DS[2], "AVERAGE");
$def[2] .= rrd::def("open", $RRDFILE[1], $DS[3], "AVERAGE");
$def[2] .= rrd::def("closed", $RRDFILE[1], $DS[4], "AVERAGE");
$def[2] .= rrd::def("canceled", $RRDFILE[1], $DS[5], "AVERAGE");
$def[2] .= rrd::def("assigned", $RRDFILE[1], $DS[6], "AVERAGE");
$def[2] .= rrd::def("faild", $RRDFILE[1], $DS[7], "AVERAGE");

$def[2] .= rrd::area("free", "#0000DD", rrd::cut("Free",12));
$def[2] .= rrd::gprint("free", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[2] .= rrd::area("open", "#FF8000", rrd::cut("Open",12), true);
$def[2] .= rrd::gprint("open", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[2] .= rrd::area("closed", "#00FF00", rrd::cut("Closed",12), true);
$def[2] .= rrd::gprint("closed", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[2] .= rrd::area("canceled", "#FFFF00", rrd::cut("Canceled",12), true);
$def[2] .= rrd::gprint("canceled", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");
$def[2] .= rrd::area("faild", "#FF0000", rrd::cut("Faild",12), true);
$def[2] .= rrd::gprint("faild", array("LAST", "AVERAGE", "MAX"), "%6.2lf%s");

# vim: set ts=4 sw=4 et syn=php :
?>
