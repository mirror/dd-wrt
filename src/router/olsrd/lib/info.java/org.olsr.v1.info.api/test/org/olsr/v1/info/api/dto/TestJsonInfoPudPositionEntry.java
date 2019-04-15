package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoPudPositionEntry {
  private JsonInfoPudPositionEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoPudPositionEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getNodeId(), equalTo(""));
    assertThat(this.impl.getDate(), equalTo(new JsonInfoPudPositionEntryDate()));
    assertThat(this.impl.getTime(), equalTo(new JsonInfoPudPositionEntryTime()));
    assertThat(this.impl.getPresent(), equalTo((Set<String>) new TreeSet<String>()));
    assertThat(Long.valueOf(this.impl.getPresentValue()), equalTo(Long.valueOf(0)));
    assertThat(this.impl.getSmask(), equalTo((Set<String>) new TreeSet<String>()));
    assertThat(Long.valueOf(this.impl.getSmaskValue()), equalTo(Long.valueOf(0)));
    assertThat(this.impl.getSig(), equalTo("INVALID"));
    assertThat(Long.valueOf(this.impl.getSigValue()), equalTo(Long.valueOf(0)));
    assertThat(this.impl.getFix(), equalTo("BAD"));
    assertThat(Long.valueOf(this.impl.getFixValue()), equalTo(Long.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getPdop()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getHdop()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getVdop()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getLatitude()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getLongitude()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getElevation()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getSpeed()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getTrack()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getMagneticTrack()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getMagneticVariation()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getSeparation()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getDgpsAge()), equalTo(Double.valueOf(0)));
    assertThat(Double.valueOf(this.impl.getDgpsSid()), equalTo(Double.valueOf(0)));
    assertThat(this.impl.getSatellites(), equalTo(new JsonInfoPudPositionEntrySatInfo()));

    /* set */
    final JsonInfoPudPositionEntryDate date = new JsonInfoPudPositionEntryDate();
    final JsonInfoPudPositionEntryTime time = new JsonInfoPudPositionEntryTime();
    final Set<String> present = new TreeSet<>();
    present.add("dummy present");
    final Set<String> smask = new TreeSet<>();
    present.add("smask present");
    final JsonInfoPudPositionEntrySatInfo satellites = new JsonInfoPudPositionEntrySatInfo();
    satellites.setInUseCount(1);
    satellites.setInViewCount(2);

    this.impl.setNodeId("nodeId");
    this.impl.setDate(date);
    this.impl.setTime(time);
    this.impl.setPresent(present);
    this.impl.setPresentValue(1);
    this.impl.setSmask(smask);
    this.impl.setSmaskValue(2);
    this.impl.setSig("SIG");
    this.impl.setSigValue(3);
    this.impl.setFix("FIX");
    this.impl.setFixValue(4);
    this.impl.setPdop(5.5);
    this.impl.setHdop(6.6);
    this.impl.setVdop(7.7);
    this.impl.setLatitude(8.8);
    this.impl.setLongitude(9.9);
    this.impl.setElevation(10.10);
    this.impl.setSpeed(11.11);
    this.impl.setTrack(12.12);
    this.impl.setMagneticTrack(13.13);
    this.impl.setMagneticVariation(14.14);
    this.impl.setSeparation(15.15);
    this.impl.setDgpsAge(16.16);
    this.impl.setDgpsSid(17);
    this.impl.setSatellites(satellites);

    /* get */
    assertThat(this.impl.getNodeId(), equalTo("nodeId"));
    assertThat(this.impl.getDate(), equalTo(date));
    assertThat(this.impl.getTime(), equalTo(time));
    assertThat(this.impl.getPresent(), equalTo(present));
    assertThat(Long.valueOf(this.impl.getPresentValue()), equalTo(Long.valueOf(1)));
    assertThat(this.impl.getSmask(), equalTo(smask));
    assertThat(Long.valueOf(this.impl.getSmaskValue()), equalTo(Long.valueOf(2)));
    assertThat(this.impl.getSig(), equalTo("SIG"));
    assertThat(Long.valueOf(this.impl.getSigValue()), equalTo(Long.valueOf(3)));
    assertThat(this.impl.getFix(), equalTo("FIX"));
    assertThat(Long.valueOf(this.impl.getFixValue()), equalTo(Long.valueOf(4)));
    assertThat(Double.valueOf(this.impl.getPdop()), equalTo(Double.valueOf(5.5)));
    assertThat(Double.valueOf(this.impl.getHdop()), equalTo(Double.valueOf(6.6)));
    assertThat(Double.valueOf(this.impl.getVdop()), equalTo(Double.valueOf(7.7)));
    assertThat(Double.valueOf(this.impl.getLatitude()), equalTo(Double.valueOf(8.8)));
    assertThat(Double.valueOf(this.impl.getLongitude()), equalTo(Double.valueOf(9.9)));
    assertThat(Double.valueOf(this.impl.getElevation()), equalTo(Double.valueOf(10.10)));
    assertThat(Double.valueOf(this.impl.getSpeed()), equalTo(Double.valueOf(11.11)));
    assertThat(Double.valueOf(this.impl.getTrack()), equalTo(Double.valueOf(12.12)));
    assertThat(Double.valueOf(this.impl.getMagneticTrack()), equalTo(Double.valueOf(13.13)));
    assertThat(Double.valueOf(this.impl.getMagneticVariation()), equalTo(Double.valueOf(14.14)));
    assertThat(Double.valueOf(this.impl.getSeparation()), equalTo(Double.valueOf(15.15)));
    assertThat(Double.valueOf(this.impl.getDgpsAge()), equalTo(Double.valueOf(16.16)));
    assertThat(Double.valueOf(this.impl.getDgpsSid()), equalTo(Double.valueOf(17)));
    assertThat(this.impl.getSatellites(), equalTo(satellites));

    this.impl.setNodeId(null);
    this.impl.setDate(null);
    this.impl.setTime(null);
    this.impl.setPresent(null);
    this.impl.setSmask(null);
    this.impl.setSig(null);
    this.impl.setFix(null);
    this.impl.setSatellites(null);

    assertThat(this.impl.getNodeId(), equalTo(""));
    assertThat(this.impl.getDate(), equalTo(new JsonInfoPudPositionEntryDate()));
    assertThat(this.impl.getTime(), equalTo(new JsonInfoPudPositionEntryTime()));
    assertThat(this.impl.getPresent(), equalTo((Set<String>) new TreeSet<String>()));
    assertThat(this.impl.getSmask(), equalTo((Set<String>) new TreeSet<String>()));
    assertThat(this.impl.getSig(), equalTo("INVALID"));
    assertThat(this.impl.getFix(), equalTo("BAD"));
    assertThat(this.impl.getSatellites(), equalTo(new JsonInfoPudPositionEntrySatInfo()));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoPudPositionEntry other;

    /* self */

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* null */

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* wrong object */

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* same */

    other = new JsonInfoPudPositionEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    /* nodeId */

    other = new JsonInfoPudPositionEntry();
    other.setNodeId("2");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* date */

    final JsonInfoPudPositionEntryDate date = new JsonInfoPudPositionEntryDate();
    date.setYear(123456);
    other = new JsonInfoPudPositionEntry();
    other.setDate(date);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* time */

    final JsonInfoPudPositionEntryTime time = new JsonInfoPudPositionEntryTime();
    time.setHour(222);
    other = new JsonInfoPudPositionEntry();
    other.setTime(time);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* present */

    final Set<String> present = new TreeSet<>();
    present.add("dummy");
    other = new JsonInfoPudPositionEntry();
    other.setPresent(present);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* presentValue */

    other = new JsonInfoPudPositionEntry();
    other.setPresentValue(42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* smask */

    final Set<String> smask = new TreeSet<>();
    smask.add("dummy");
    other = new JsonInfoPudPositionEntry();
    other.setSmask(smask);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* smaskValue */

    other = new JsonInfoPudPositionEntry();
    other.setSmaskValue(42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* sig */

    other = new JsonInfoPudPositionEntry();
    other.setSig("2");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* sigValue */

    other = new JsonInfoPudPositionEntry();
    other.setSigValue(42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* fix */

    other = new JsonInfoPudPositionEntry();
    other.setFix("2");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* fixValue */

    other = new JsonInfoPudPositionEntry();
    other.setFixValue(42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* pdop */

    other = new JsonInfoPudPositionEntry();
    other.setPdop(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* hdop */

    other = new JsonInfoPudPositionEntry();
    other.setHdop(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* vdop */

    other = new JsonInfoPudPositionEntry();
    other.setVdop(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* latitude */

    other = new JsonInfoPudPositionEntry();
    other.setLatitude(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* longitude */

    other = new JsonInfoPudPositionEntry();
    other.setLongitude(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* elevation */

    other = new JsonInfoPudPositionEntry();
    other.setElevation(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* speed */

    other = new JsonInfoPudPositionEntry();
    other.setSpeed(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* track */

    other = new JsonInfoPudPositionEntry();
    other.setTrack(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* magneticTrack */

    other = new JsonInfoPudPositionEntry();
    other.setMagneticTrack(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* magneticVariation */

    other = new JsonInfoPudPositionEntry();
    other.setMagneticVariation(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* separation */

    other = new JsonInfoPudPositionEntry();
    other.setSeparation(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* dgpsAge */

    other = new JsonInfoPudPositionEntry();
    other.setDgpsAge(42.42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* dgpsSid */

    other = new JsonInfoPudPositionEntry();
    other.setDgpsSid(42);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    /* satellites */

    final JsonInfoPudPositionEntrySatInfo sats = new JsonInfoPudPositionEntrySatInfo();
    sats.setInUseCount(111);
    other = new JsonInfoPudPositionEntry();
    other.setSatellites(sats);
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(62468262)));

    /* set */
    final JsonInfoPudPositionEntryDate date = new JsonInfoPudPositionEntryDate();
    final JsonInfoPudPositionEntryTime time = new JsonInfoPudPositionEntryTime();
    final Set<String> present = new TreeSet<>();
    present.add("dummy present");
    final Set<String> smask = new TreeSet<>();
    present.add("smask present");
    final JsonInfoPudPositionEntrySatInfo satellites = new JsonInfoPudPositionEntrySatInfo();
    satellites.setInUseCount(1);
    satellites.setInViewCount(2);

    this.impl.setNodeId("nodeId");
    this.impl.setDate(date);
    this.impl.setTime(time);
    this.impl.setPresent(present);
    this.impl.setPresentValue(1);
    this.impl.setSmask(smask);
    this.impl.setSmaskValue(2);
    this.impl.setSig("SIG");
    this.impl.setSigValue(3);
    this.impl.setFix("FIX");
    this.impl.setFixValue(4);
    this.impl.setPdop(5.5);
    this.impl.setHdop(6.6);
    this.impl.setVdop(7.7);
    this.impl.setLatitude(8.8);
    this.impl.setLongitude(9.9);
    this.impl.setElevation(10.10);
    this.impl.setSpeed(11.11);
    this.impl.setTrack(12.12);
    this.impl.setMagneticTrack(13.13);
    this.impl.setMagneticVariation(14.14);
    this.impl.setSeparation(15.15);
    this.impl.setDgpsAge(16.16);
    this.impl.setDgpsSid(17);
    this.impl.setSatellites(satellites);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(540137580)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}