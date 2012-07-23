<?php
#
# Monitoring Plugin PNP - check_akcp.php
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

foreach ($this->DS as $KEY=>$VAL) {
    list($type, $name) = explode('_', $VAL['NAME']);
    $unit = $VAL['UNIT'];

    if ($type == 'temp') {
        $unit = "°$unit";
        $ds_name[$KEY] = "Temperatur $name";
        $opt[$KEY] = "--vertical-label \"Temperatur in $unit\" --title \"Temperatur $hostname / $name\" ";

        $def[$KEY]  = rrd::def("temp", $VAL['RRDFILE'], $VAL['DS'], "AVERAGE");
        $def[$KEY] .= rrd::gradient("temp", "A55231", "DEC6BD", rrd::cut($name,16), 20);
        $def[$KEY] .= rrd::gprint("temp", array("LAST","MAX","AVERAGE"), "%5.1lf $unit");
        $def[$KEY] .= rrd::line1("temp", "#000000");
    } else if ($type == 'hum') {
        $unit = "%";
        $ds_name[$KEY] = "Humidity $name";
        $opt[$KEY] = "--vertical-label \"Humidity in %\" --title \"Humidity $hostname / $name\" ";

        $def[$KEY]  = rrd::def("hum", $VAL['RRDFILE'], $VAL['DS'], "AVERAGE");
        $def[$KEY] .= rrd::gradient("hum", "3152A5", "BDC6DE", rrd::cut($name,16), 20);
        $def[$KEY] .= rrd::gprint("hum", array("LAST","MAX","AVERAGE"), "%5.1lf%%");
        $def[$KEY] .= rrd::line1("hum", "#000000");
    }

    $def[$KEY] .= rrd::hrule($VAL['WARN_MIN'], "#FFFF00", "Warning  lower\:  ".$VAL['WARN_MIN']."$unit upper\:  ".$VAL['WARN_MAX']."$unit \\n");
    $def[$KEY] .= rrd::hrule($VAL['WARN_MAX'], "#FFFF00");
    $def[$KEY] .= rrd::hrule($VAL['CRIT_MIN'], "#FF0000", "Critical lower\:  ".$VAL['CRIT_MIN']."$unit upper\:  ".$VAL['CRIT_MAX']."$unit  \\n");
    $def[$KEY] .= rrd::hrule($VAL['CRIT_MIN'], "#FF0000");
}

# vim: set ts=4 sw=4 et syn=php :
?>
