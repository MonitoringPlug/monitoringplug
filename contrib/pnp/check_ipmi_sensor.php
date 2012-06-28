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

#
# Define some colors ..
#
$_WARNRULE = '#FFFF00';
$_CRITRULE = '#FF0000';
$_AREA     = '#256aef';
$_AREA2    = '#BDC6DE';
$_LINE     = '#000000';

foreach ($this->DS as $KEY=>$VAL) {

    $unit = $VAL['UNIT'];
    $vlabel = "";

    if ($unit == "volts") {
        $unit = "V";
        $vlabel = "Volts";
    } else if ($unit == "watts") {
        $unit = "W";
        $vlabel = "Watts";
    } else if ($unit == "%%") {
        $unit = "%";
        $vlabel = "Percentage";
    } else if ($unit == "C") {
        $unit = "°C";
        $vlabel = "Centigrade";
    } else {
        $vlabel = $unit;
    }

    // Graph Name
    $ds_name[$KEY] = $VAL['LABEL'];

    // Graph Opt
    $opt[$KEY] = '--vertical-label "' . $vlabel . '" --title "' . $this->MACRO['DISP_HOSTNAME'] . ' / ' . $VAL['LABEL'] . '"';
    if ($unit == "%") {
        $opt[$KEY] .= " --lower=0 --upper=101 ";
    }

    // Draw
    $def[$KEY]  = rrd::def("var1", $VAL['RRDFILE'], $VAL['DS'], "AVERAGE");
    $def[$KEY] .= rrd::gradient("var1", $_AREA, $_AREA2, rrd::cut($VAL['NAME'],16), 20);
    $def[$KEY] .= rrd::line1("var1", $_LINE );
    $def[$KEY] .= rrd::gprint("var1", array("LAST","MAX","AVERAGE"), "%3.4lf%S$unit");

    // Warning Lines
    if ($VAL['WARN'] != "") {
        $def[$KEY] .= rrd::hrule($VAL['WARN'], $_WARNRULE, "Warning  ".$VAL['WARN']."$unit \\n");
    } else if ($VAL['WARN_MIN'] != "" && $VAL['WARN_MAX'] != "") {
        $def[$KEY] .= rrd::hrule($VAL['WARN_MIN'], $_WARNRULE, "Warning  (min\\: ".$VAL['WARN_MIN']."$unit  min\\: ".$VAL['WARN_MAX']."$unit)\\n");
        $def[$KEY] .= rrd::hrule($VAL['WARN_MAX'], $_WARNRULE);
    }

    // Critical Lines
    if ($VAL['CRIT'] != "") {
        $def[$KEY] .= rrd::hrule($VAL['CRIT'], $_CRITRULE, "Warning  ".$VAL['CRIT']."$unit \\n");
    } else if ($VAL['CRIT_MIN'] != "" && $VAL['CRIT_MAX'] != "") {
        $def[$KEY] .= rrd::hrule($VAL['CRIT_MIN'], $_CRITRULE, "Warning  (min\\: ".$VAL['CRIT_MIN']."$unit  min\\: ".$VAL['CRIT_MAX']."$unit)\\n");
        $def[$KEY] .= rrd::hrule($VAL['CRIT_MAX'], $_CRITRULE);
    }
}

# vim: set ts=4 sw=4 et syn=php :
?>
