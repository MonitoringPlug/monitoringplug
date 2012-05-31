<?php
#
# Monitoring Plugin PNP - check_nfs.php
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

# Some Config
$COLORS = array(
    "tcp_v3"  => "#800000", "tcp_v4"  => "#FF0000",
    "udp_v3"  => "#008000", "udp_v4"  => "#00FF00",
    "tcp6_v3" => "#000080", "tcp6_v4" => "#0000FF",
    "udp6_v3" => "#804000", "udp6_v4" => "#FF8000"
);
$NAMES = array(
    "tcp_v3"  => "NFSv3 TCP",  "tcp_v4"  => "NFSv4 TCP",
    "udp_v3"  => "NFSv3 UDP",  "udp_v4"  => "NFSv4 UDP",
    "tcp6_v3" => "NFSv3 TCP6", "tcp6_v4" => "NFSv4 TCP6",
    "udp6_v3" => "NFSv3 UDP6", "udp6_v4" => "NFSv4 UDP6"
);

$opt[1] = "--vertical-label \"$UNIT[1]\" --title \"Response Times $hostname / $servicedesc\" ";

$ds_name[1] = "NFS Response Time";

$def[1] = "";

foreach ($this->DS as $KEY=>$VAL) {
    $def[1] .= rrd::def("var_$KEY", $VAL['RRDFILE'], $VAL['DS'], "AVERAGE");
    $def[1] .= rrd::line1("var_$KEY", $COLORS[$VAL['NAME']] , rrd::cut($NAMES[$VAL['NAME']],12));
    $def[1].= rrd::gprint  ("var_$KEY", array("LAST","MAX","AVERAGE"), "%6.3lf %S".$VAL['UNIT']);
}

# vim: set ts=4 sw=4 et syn=php :
?>
