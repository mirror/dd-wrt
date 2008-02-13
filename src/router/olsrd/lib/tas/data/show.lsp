<?lua

 olsr_state = tas.olsr_get_info()

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
  <head>
    <title>
      OLSR State
    </title>
  </head>
  <body>

    <h2>Links</h2>

    <table border="1">
      <tr>
        <td>
          <b>Local</b>
        </td>
        <td>
          <b>Remote</b>
        </td>
        <td>
          <b>Main</b>
        </td>
        <td>
          <b>Hysteresis</b>
        </td>
        <td>
          <b>LQ</b>
        </td>
        <td>
          <b>NLQ</b>
        </td>
        <td>
          <b>ETX</b>
        </td>
      <tr>

<?lua

i = 0

while olsr_state["links"][i] do

?>

      <tr>
        <td>
          <?lua tas.write(olsr_state["links"][i]["local"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["links"][i]["remote"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["links"][i]["main"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["links"][i]["hysteresis"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["links"][i]["lq"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["links"][i]["nlq"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["links"][i]["etx"]) ?>
        </td>
      </tr>

<?lua

i = i + 1

end

?>

    </table>

    <h2>Neighbors</h2>

    <table border="1">
      <tr>
        <td>
          <b>Main</b>
        </td>
        <td>
          <b>Symmetric</b>
        </td>
        <td>
          <b>MPR</b>
        </td>
        <td>
          <b>MPRS</b>
        </td>
        <td>
          <b>Willingness</b>
        </td>
        <td>
          <b>2-Hop Neighbors</b>
        </td>
      <tr>

<?lua

i = 0

while olsr_state["neighbors"][i] do

?>

      <tr>
        <td>
          <?lua tas.write(olsr_state["neighbors"][i]["main"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["neighbors"][i]["symmetric"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["neighbors"][i]["mpr"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["neighbors"][i]["mprs"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["neighbors"][i]["willingness"]) ?>
        </td>
        <td>

          <?lua

           k = 0

           while olsr_state["neighbors"][i]["neighbors2"][k] do

            tas.write(olsr_state["neighbors"][i]["neighbors2"][k])

          ?>

          <br>

          <?lua

            k = k + 1

           end

          ?>

        </td>
      </tr>

<?lua

i = i + 1

end

?>

    </table>

    <h2>Topology</h2>

    <table border="1">
      <tr>
        <td>
          <b>Main</b>
        </td>
        <td>
          <b>Destination</b>
        </td>
        <td>
          <b>ETX</b>
        </td>
      <tr>

<?lua

i = 0

while olsr_state["topology"][i] do

?>

      <tr>
        <td>
          <?lua tas.write(olsr_state["topology"][i]["main"]) ?>
        </td>
        <td>

          <?lua

           k = 0

           while olsr_state["topology"][i]["destinations"][k] do

            tas.write(olsr_state["topology"][i]["destinations"][k]["address"])

          ?>

          <br>

          <?lua

            k = k + 1

           end

          ?>

        </td>
        <td>

          <?lua

           k = 0

           while olsr_state["topology"][i]["destinations"][k] do

            tas.write(olsr_state["topology"][i]["destinations"][k]["etx"])

          ?>

          <br>

          <?lua

            k = k + 1

           end

          ?>

        </td>
      </tr>

<?lua

i = i + 1

end

?>

    </table>

    <h2>Routes</h2>

    <table border="1">
      <tr>
        <td>
          <b>Destination</b>
        </td>
        <td>
          <b>Gateway</b>
        </td>
        <td>
          <b>Interface</b>
        </td>
        <td>
          <b>Metric</b>
        </td>
      <tr>

<?lua

i = 0

while olsr_state["routes"][i] do

?>

      <tr>
        <td>
          <?lua tas.write(olsr_state["routes"][i]["destination"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["routes"][i]["gateway"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["routes"][i]["interface"]) ?>
        </td>
        <td>
          <?lua tas.write(olsr_state["routes"][i]["metric"]) ?>
        </td>
      </tr>

<?lua

i = i + 1

end

?>

    </table>

  </body>
</html>
