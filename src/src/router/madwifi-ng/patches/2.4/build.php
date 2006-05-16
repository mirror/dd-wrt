<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
    <HTML>
    <HEAD>
    <TITLE>Support-Portal</TITLE>
    <meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">
    </HEAD>
    <BODY class="normal">
    <br><br><br>
    <form method="post" action="build.php" name="focus">
    
    <br>
    
<?php
$action=$_POST['action'];
$mac=$_POST['mac'];
$build=$_POST['build_type'];

switch($action)
{
default:
?>
    <select name="build_type">
    <option value="standard">standard</option>
    <option value="kbits">kbits control</option>
    </select>
    <div>MAC Address</div>
    <input type="text" size="17" name="mac" />
    <input type="submit" name="action" value="build" />
</form>
</body>
</html>

<?php
break;
case "build":
$mac = strtoupper($mac);
//00:00:00:00:00:00
if (strlen($mac)!=17)
    {
    printf("mac is wrong, please check input format (00:00:00:00:00:00)<br>");
    flush();
    exit();
    }
printf("preparing filesystem....<br>");
flush();
$target=strtr($mac,":","_");

exec("mkdir ./build".$target);
exec("cp -r ./rootfs_wts".$build."/* ./build".$target);
exec("sed -i -e 's/01:02:03:04:05:06/".$mac."/' ./build".$target."/sbin/rc");
exec("sed -i -e 's/01:02:03:04:05:06/".$mac."/' ./build".$target."/usr/sbin/httpd");
printf("building binary.... (this may take a while)<br>");
flush();
exec("./bin/mksquashfs-lzma ./build".$target." target".$target.".squashfs -noappend -root-owned -le");
exec("rm -rf ./build".$target);
exec("./bin/trx -o binary".$target.".bin ./loader-0.02/loader.gz ./kernel/vmlinuz target".$target.".squashfs");
exec("rm target".$target.".squashfs");
printf("submit to download...<br>");
flush();
printf("<a href=\"binary".$target.".bin\">download</a><br>");
printf("<a href=\"build.php\">back to mac generation site</a><br>");

flush();
break;
}

?>
