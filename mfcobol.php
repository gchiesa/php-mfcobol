<?php
/**
* Copyright 2007, 2008 - Giuseppe Chiesa
*
* This file is part of mfcobol.
*
* mfcobol is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* mfcobol is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with sp2html; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Created by: Giuseppe Chiesa - http://gchiesa.smos.org
*
**/




$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('mfcobol')) {
	dl('mfcobol.' . PHP_SHLIB_SUFFIX);
}
$module = 'mfcobol';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = 'confirm_' . $module . '_compiled';
if (extension_loaded($module)) {
	$str = $function($module);
} else {
	$str = "Module $module is not compiled into PHP";
}
echo "$str\n";

echo "\ntesting functions:\n";

echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_comp3encode ---\n";
$data = "20080812";
echo "DUMP_COMP3_ENCODE OF ".$data." : ".bin2hex(mfcdb_comp3encode($data, MFC_COMP3_TYPE_F, 10))."\n";
echo "DUMP_COMP3_ENCODE OF ".$data." : ".bin2hex(mfcdb_comp3encode($data, MFC_COMP3_TYPE_F))."\n";
echo "DUMP_COMP3_ENCODE OF ".$data." : ".bin2hex(mfcdb_comp3encode($data, MFC_COMP3_TYPE_F, 11))."\n";

echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_comp3decode ---\n";
$data = "\x00\x00\x00\x00\x00\x00\x00\x0C";
echo "COMP3 DECODE OF ".bin2hex($data)." -> ".mfcdb_comp3decode($data, 8)."\n";


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_open ---\n";
$res = mfcdb_open("test");
echo "RESOURCE_ID(test): ".$res."\n";
$res2 = mfcdb_open("mgchiart");
echo "RESOURCE_ID(mgchiart): ".$res2."\n";


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_get_info ---\n";
$info = mfcdb_get_info($res);
echo "DUMP_INFO(test): ".print_r($info, true)."\n";
$info = mfcdb_get_info($res2);
echo "DUMP_INFO(test2): ".print_r($info, true)."\n";


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_fetch ---\n";
$row = mfcdb_fetch($res, "0301006", 0);
echo "on table TEST\n";
echo "DUMP_ROW: |".$row."|\n";
echo "DUMP_(bin2hex): |".bin2hex($row)."|\n";
$row = mfcdb_fetch($res2, "XXXACLSRL0001");
echo "on table mgchiart\n";
echo "DUMP_ROW: |".$row."|\n";
echo "DUMP_(bin2hex): |".bin2hex($row)."|\n";


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_start ---\n";
echo "on table mgchiart\n";
$row = mfcdb_start($res2, 0, "XXXACLSRL0", MFCDB_ISGTEQ);
if(!$row) { 
    echo "RETURN VALUE FALSE\n";
} else {
    echo "RETURN VALUE TRUE\n";
}
echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_curr ---\n";
echo "getting curr record\n";
$row = mfcdb_curr($res2);
echo "DUMP_ROW: |".$row."|\n";
echo "DUMP_(bin2hex): |".bin2hex($row)."|\n";
echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_next ---\n";
echo "getting next record\n";
$row = mfcdb_next($res2);
echo "DUMP_ROW: |".$row."|\n";
echo "DUMP_(bin2hex): |".bin2hex($row)."|\n";
echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_prev ---\n";
echo "getting prev record\n";
$row = mfcdb_prev($res2);
echo "DUMP_ROW: |".$row."|\n";
echo "DUMP_(bin2hex): |".bin2hex($row)."|\n";
echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_prev ---\n";
echo "getting prev record\n";
$row = mfcdb_prev($res2);
echo "DUMP_ROW: |".$row."|\n";
echo "DUMP_(bin2hex): |".bin2hex($row)."|\n";

echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_start2 ---\n";
echo "on table mgchicaa\n";
$res3 = mfcdb_open('/media/sdb9/usr/sp3/arc/chi/mgchicaa');
$row = mfcdb_start($res3, 2, "SUK2213110   ", MFC_ISGTEQ);
if(!$row) { 
    echo "RETURN VALUE FALSE\n";
    } else {
        echo "RETURN VALUE TRUE\n";
        }
$row = mfcdb_curr($res3);
echo "DUMP_ROW: |".$row."|\n";
echo "DUMP_(bin2hex): |".bin2hex($row)."|\n";
mfcdb_close($res3);
        

echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_exists ---\n";
$row = mfcdb_exists($res, "0401005");
echo "DUMP_EXISTS (YES): ".(($row)?'TRUE':'FALSE')."\n";
$row = mfcdb_exists($res, "9901005");
echo "DUMP_EXISTS (NO): ".(($row)?'TRUE':'FALSE')."\n";


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_insert ---\n";
$row = mfcdb_fetch($res, "0301001", 0);
$newrow = "9901005".substr($row, 7, 136);
echo "INSERTING ROW: |".$newrow."| LEN: ".strlen($newrow)."\n";
$iResult = mfcdb_insert($res, $newrow);
echo "Return value : ".$iResult."\n";


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_error ---\n";
if($iResult == FALSE) {
    echo "Return string error: ".mfcdb_error()."\n";
}

$row = mfcdb_exists($res, "9901005", 0);
echo "DUMP_EXISTS (NO): ".(($row)?'TRUE':'FALSE')."\n";


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_replace ---\n";
$newrow = "9801005".substr($newrow, 7, 136);
echo "REPLACEING ROW 9901005 WITH: ".$newrow."\n";
$iResult = mfcdb_replace($res, "9901005", $newrow);
echo "Return value : ".$iResult.", replaced recnum : ".mfcdb_get_recnum()."\n";
if($iResult == FALSE) {
    echo "Return string error (".$iResult.") : ".mfcdb_error()."\n";
}


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_delete ---\n";
echo "DELETING ROW 9801005\n";
$iResult = mfcdb_delete($res, "9801005");
echo "Return value : ".$iResult.", deleted recnum : ".mfcdb_get_recnum()."\n";
if($iResult == FALSE) {
    echo "Return string error (".$iResult.") : ".mfcdb_error()."\n";
}


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_close ---\n";
$iResult = mfcdb_close($res);
echo "Return value: ".$iResult."\n";
$iResult = mfcdb_close($res2);
echo "Return value(mgchiart): ".$iResult."\n";

echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_key ---\n";
$keyres = mfcdb_key(MFC_ISNODUPS, 1, 0, 7, MFC_CHARTYPE);
if(!$keyres) { 
    echo "RETURN VALUE FALSE\n";
} else {
    echo "RETURN VALUE TRUE\n";
}


echo "\n---===---===---===---===---===---\nFUNCTION: --- mfcdb_create ---\n";
if($keyres) {
    $res = mfcdb_create("mydb", 20, $keyres);
    echo "Res is ".$res."\n";
    $row = sprintf("%20s", "0101001Giuseppe Chie");
    echo "insert |".$row."|\n";
    $iResult = mfcdb_insert($res, $row);
    $row = sprintf("%20s", "0110005Francesco Chi");
    $iResult = mfcdb_insert($res, $row);
    
    $iResult = mfcdb_close($res);
    echo "Return value: ".$iResult."\n";
}

   
    
echo "*** END ***\n\n";



?>
