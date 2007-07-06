<?
/*
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**************************************************
 * CONFIGURATION
 **************************************************/
$db_host = "localhost";
$db_user = "bmon";
$db_password = "bmon";
$db_name = "bmon";
/**************************************************
 * END CONFIGURATION
 **************************************************/

function find_divisor($max)
{
	if ($max >= 1073741824)
		return 1073741824;
	else if ($max >= 1048576)
		return 1048576;
	else if ($max > 1024)
		return 1024;
	else
		return 1;
}

function find_unit($div)
{
	if ($div == 1073741824)
		return "GiB";
	else if ($div == 1048576)
		return "MiB";
	else if ($div == 1024)
		return "KiB";
	else
		return "B";
}

?>
