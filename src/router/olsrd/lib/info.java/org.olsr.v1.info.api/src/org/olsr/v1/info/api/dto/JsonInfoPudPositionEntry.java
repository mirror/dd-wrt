package org.olsr.v1.info.api.dto;

import java.util.Set;
import java.util.TreeSet;

import org.olsr.v1.info.api.commands.InfoCommand;
import org.osgi.annotation.versioning.ProviderType;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * A pud position entry in the {@link InfoCommand#PUD_POSITION} jsoninfo OLSRd plugin response
 */
@ProviderType
public class JsonInfoPudPositionEntry {
  private String                          nodeId            = "";
  private JsonInfoPudPositionEntryDate    date              = new JsonInfoPudPositionEntryDate();
  private JsonInfoPudPositionEntryTime    time              = new JsonInfoPudPositionEntryTime();
  private final Set<String>               present           = new TreeSet<>();
  private long                            presentValue      = 0;
  private final Set<String>               smask             = new TreeSet<>();
  private long                            smaskValue        = 0;
  private String                          sig               = "INVALID";
  private long                            sigValue          = 0;
  private String                          fix               = "BAD";
  private long                            fixValue          = 0;
  private double                          pdop              = 0.0;
  private double                          hdop              = 0.0;
  private double                          vdop              = 0.0;
  private double                          latitude          = 0.0;
  private double                          longitude         = 0.0;
  private double                          elevation         = 0.0;
  private double                          speed             = 0.0;
  private double                          track             = 0.0;
  private double                          magneticTrack     = 0.0;
  private double                          magneticVariation = 0.0;
  private double                          separation        = 0.0;
  private double                          dgpsAge           = 0.0;
  private long                            dgpsSid           = 0;
  private JsonInfoPudPositionEntrySatInfo satellites        = new JsonInfoPudPositionEntrySatInfo();

  /**
   * @return the node id
   */
  public String getNodeId() {
    return this.nodeId;
  }

  /**
   * @param nodeId the node id to set
   */
  @JsonProperty("nodeId")
  public void setNodeId(final String nodeId) {
    if (nodeId == null) {
      this.nodeId = "";
    } else {
      this.nodeId = nodeId;
    }
  }

  /**
   * @return the date
   */
  public JsonInfoPudPositionEntryDate getDate() {
    return this.date;
  }

  /**
   * @param date the date to set
   */
  @JsonProperty("date")
  public void setDate(final JsonInfoPudPositionEntryDate date) {
    if (date == null) {
      this.date = new JsonInfoPudPositionEntryDate();
    } else {
      this.date = date;
    }
  }

  /**
   * @return the time
   */
  public JsonInfoPudPositionEntryTime getTime() {
    return this.time;
  }

  /**
   * @param time the time to set
   */
  @JsonProperty("time")
  public void setTime(final JsonInfoPudPositionEntryTime time) {
    if (time == null) {
      this.time = new JsonInfoPudPositionEntryTime();
    } else {
      this.time = time;
    }
  }

  /**
   * @return the present mask
   */
  public Set<String> getPresent() {
    return this.present;
  }

  /**
   * @param present the present mask to set
   */
  @JsonProperty("present")
  public void setPresent(final Set<String> present) {
    this.present.clear();
    if (present != null) {
      this.present.addAll(present);
    }
  }

  /**
   * @return the present mask value
   */
  public long getPresentValue() {
    return this.presentValue;
  }

  /**
   * @param presentValue the present mask value to set
   */
  @JsonProperty("presentValue")
  public void setPresentValue(final long presentValue) {
    this.presentValue = presentValue;
  }

  /**
   * @return the sentence mask
   */
  public Set<String> getSmask() {
    return this.smask;
  }

  /**
   * @param smask the sentence mask to set
   */
  @JsonProperty("smask")
  public void setSmask(final Set<String> smask) {
    this.smask.clear();
    if (smask != null) {
      this.smask.addAll(smask);
    }
  }

  /**
   * @return the sentence mask value
   */
  public long getSmaskValue() {
    return this.smaskValue;
  }

  /**
   * @param smaskValue the sentence mask value to set
   */
  @JsonProperty("smaskValue")
  public void setSmaskValue(final long smaskValue) {
    this.smaskValue = smaskValue;
  }

  /**
   * @return the signal status
   */
  public String getSig() {
    return this.sig;
  }

  /**
   * @param sig the signal status to set
   */
  @JsonProperty("sig")
  public void setSig(final String sig) {
    if (sig == null) {
      this.sig = "INVALID";
    } else {
      this.sig = sig;
    }
  }

  /**
   * @return the signal status value
   */
  public long getSigValue() {
    return this.sigValue;
  }

  /**
   * @param sigValue the signal status value to set
   */
  @JsonProperty("sigValue")
  public void setSigValue(final long sigValue) {
    this.sigValue = sigValue;
  }

  /**
   * @return the fix status
   */
  public String getFix() {
    return this.fix;
  }

  /**
   * @param fix the fix status to set
   */
  @JsonProperty("fix")
  public void setFix(final String fix) {
    if (fix == null) {
      this.fix = "BAD";
    } else {
      this.fix = fix;
    }
  }

  /**
   * @return the fix status value
   */
  public long getFixValue() {
    return this.fixValue;
  }

  /**
   * @param fixValue the fix status value to set
   */
  @JsonProperty("fixValue")
  public void setFixValue(final long fixValue) {
    this.fixValue = fixValue;
  }

  /**
   * @return the pdop
   */
  public double getPdop() {
    return this.pdop;
  }

  /**
   * @param pdop the pdop to set
   */
  @JsonProperty("pdop")
  public void setPdop(final double pdop) {
    this.pdop = pdop;
  }

  /**
   * @return the hdop
   */
  public double getHdop() {
    return this.hdop;
  }

  /**
   * @param hdop the hdop to set
   */
  @JsonProperty("hdop")
  public void setHdop(final double hdop) {
    this.hdop = hdop;
  }

  /**
   * @return the vdop
   */
  public double getVdop() {
    return this.vdop;
  }

  /**
   * @param vdop the vdop to set
   */
  @JsonProperty("vdop")
  public void setVdop(final double vdop) {
    this.vdop = vdop;
  }

  /**
   * @return the latitude
   */
  public double getLatitude() {
    return this.latitude;
  }

  /**
   * @param latitude the latitude to set
   */
  @JsonProperty("latitude")
  public void setLatitude(final double latitude) {
    this.latitude = latitude;
  }

  /**
   * @return the longitude
   */
  public double getLongitude() {
    return this.longitude;
  }

  /**
   * @param longitude the longitude to set
   */
  @JsonProperty("longitude")
  public void setLongitude(final double longitude) {
    this.longitude = longitude;
  }

  /**
   * @return the elevation
   */
  public double getElevation() {
    return this.elevation;
  }

  /**
   * @param elevation the elevation to set
   */
  @JsonProperty("elevation")
  public void setElevation(final double elevation) {
    this.elevation = elevation;
  }

  /**
   * @return the speed
   */
  public double getSpeed() {
    return this.speed;
  }

  /**
   * @param speed the speed to set
   */
  @JsonProperty("speed")
  public void setSpeed(final double speed) {
    this.speed = speed;
  }

  /**
   * @return the track
   */
  public double getTrack() {
    return this.track;
  }

  /**
   * @param track the track to set
   */
  @JsonProperty("track")
  public void setTrack(final double track) {
    this.track = track;
  }

  /**
   * @return the magnetic track
   */
  public double getMagneticTrack() {
    return this.magneticTrack;
  }

  /**
   * @param magneticTrack the magnetic track to set
   */
  @JsonProperty("magneticTrack")
  public void setMagneticTrack(final double magneticTrack) {
    this.magneticTrack = magneticTrack;
  }

  /**
   * @return the magnetic variation
   */
  public double getMagneticVariation() {
    return this.magneticVariation;
  }

  /**
   * @param magneticVariation the magnetic variation to set
   */
  @JsonProperty("magneticVariation")
  public void setMagneticVariation(final double magneticVariation) {
    this.magneticVariation = magneticVariation;
  }

  /**
   * @return the separation
   */
  public double getSeparation() {
    return this.separation;
  }

  /**
   * @param separation the separation to set
   */
  @JsonProperty("separation")
  public void setSeparation(final double separation) {
    this.separation = separation;
  }

  /**
   * @return the dgpsAge
   */
  public double getDgpsAge() {
    return this.dgpsAge;
  }

  /**
   * @param dgpsAge the dgpsAge to set
   */
  @JsonProperty("dgpsage")
  public void setDgpsAge(final double dgpsAge) {
    this.dgpsAge = dgpsAge;
  }

  /**
   * @return the dgpsSid
   */
  public long getDgpsSid() {
    return this.dgpsSid;
  }

  /**
   * @param dgpsSid the dgpsSid to set
   */
  @JsonProperty("dgpssid")
  public void setDgpsSid(final long dgpsSid) {
    this.dgpsSid = dgpsSid;
  }

  /**
   * @return the satellites
   */
  public JsonInfoPudPositionEntrySatInfo getSatellites() {
    return this.satellites;
  }

  /**
   * @param satellites the satellites to set
   */
  @JsonProperty("satellites")
  public void setSatellites(final JsonInfoPudPositionEntrySatInfo satellites) {
    if (satellites == null) {
      this.satellites = new JsonInfoPudPositionEntrySatInfo();
    } else {
      this.satellites = satellites;
    }
  }

  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    long temp;
    result = (prime * result) + this.nodeId.hashCode();
    result = (prime * result) + this.date.hashCode();
    result = (prime * result) + this.time.hashCode();
    result = (prime * result) + this.present.hashCode();
    result = (prime * result) + (int) (this.presentValue ^ (this.presentValue >>> 32));
    result = (prime * result) + this.smask.hashCode();
    result = (prime * result) + (int) (this.smaskValue ^ (this.smaskValue >>> 32));
    result = (prime * result) + this.sig.hashCode();
    result = (prime * result) + (int) (this.sigValue ^ (this.sigValue >>> 32));
    result = (prime * result) + this.fix.hashCode();
    result = (prime * result) + (int) (this.fixValue ^ (this.fixValue >>> 32));
    temp = Double.doubleToLongBits(this.pdop);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.hdop);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.vdop);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.latitude);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.longitude);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.elevation);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.speed);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.track);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.magneticTrack);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.magneticVariation);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.separation);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    temp = Double.doubleToLongBits(this.dgpsAge);
    result = (prime * result) + (int) (temp ^ (temp >>> 32));
    result = (prime * result) + (int) (this.dgpsSid ^ (this.dgpsSid >>> 32));
    result = (prime * result) + this.satellites.hashCode();
    return result;
  }

  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }
    if (obj == null) {
      return false;
    }
    if (this.getClass() != obj.getClass()) {
      return false;
    }
    final JsonInfoPudPositionEntry other = (JsonInfoPudPositionEntry) obj;
    if (!this.nodeId.equals(other.nodeId)) {
      return false;
    }
    if (!this.date.equals(other.date)) {
      return false;
    }
    if (!this.time.equals(other.time)) {
      return false;
    }
    if (!this.present.equals(other.present)) {
      return false;
    }
    if (this.presentValue != other.presentValue) {
      return false;
    }
    if (!this.smask.equals(other.smask)) {
      return false;
    }
    if (this.smaskValue != other.smaskValue) {
      return false;
    }
    if (!this.sig.equals(other.sig)) {
      return false;
    }
    if (this.sigValue != other.sigValue) {
      return false;
    }
    if (!this.fix.equals(other.fix)) {
      return false;
    }
    if (this.fixValue != other.fixValue) {
      return false;
    }
    if (Double.doubleToLongBits(this.pdop) != Double.doubleToLongBits(other.pdop)) {
      return false;
    }
    if (Double.doubleToLongBits(this.hdop) != Double.doubleToLongBits(other.hdop)) {
      return false;
    }
    if (Double.doubleToLongBits(this.vdop) != Double.doubleToLongBits(other.vdop)) {
      return false;
    }
    if (Double.doubleToLongBits(this.latitude) != Double.doubleToLongBits(other.latitude)) {
      return false;
    }
    if (Double.doubleToLongBits(this.longitude) != Double.doubleToLongBits(other.longitude)) {
      return false;
    }
    if (Double.doubleToLongBits(this.elevation) != Double.doubleToLongBits(other.elevation)) {
      return false;
    }
    if (Double.doubleToLongBits(this.speed) != Double.doubleToLongBits(other.speed)) {
      return false;
    }
    if (Double.doubleToLongBits(this.track) != Double.doubleToLongBits(other.track)) {
      return false;
    }
    if (Double.doubleToLongBits(this.magneticTrack) != Double.doubleToLongBits(other.magneticTrack)) {
      return false;
    }
    if (Double.doubleToLongBits(this.magneticVariation) != Double.doubleToLongBits(other.magneticVariation)) {
      return false;
    }
    if (Double.doubleToLongBits(this.separation) != Double.doubleToLongBits(other.separation)) {
      return false;
    }
    if (Double.doubleToLongBits(this.dgpsAge) != Double.doubleToLongBits(other.dgpsAge)) {
      return false;
    }
    if (this.dgpsSid != other.dgpsSid) {
      return false;
    }
    if (!this.satellites.equals(other.satellites)) {
      return false;
    }
    return true;
  }

  @Override
  public String toString() {
    final StringBuilder builder = new StringBuilder();
    builder.append("JsonInfoPudPositionEntry [nodeId=");
    builder.append(this.nodeId);
    builder.append(", date=");
    builder.append(this.date);
    builder.append(", time=");
    builder.append(this.time);
    builder.append(", present=");
    builder.append(this.present);
    builder.append(", presentValue=");
    builder.append(this.presentValue);
    builder.append(", smask=");
    builder.append(this.smask);
    builder.append(", smaskValue=");
    builder.append(this.smaskValue);
    builder.append(", sig=");
    builder.append(this.sig);
    builder.append(", sigValue=");
    builder.append(this.sigValue);
    builder.append(", fix=");
    builder.append(this.fix);
    builder.append(", fixValue=");
    builder.append(this.fixValue);
    builder.append(", pdop=");
    builder.append(this.pdop);
    builder.append(", hdop=");
    builder.append(this.hdop);
    builder.append(", vdop=");
    builder.append(this.vdop);
    builder.append(", latitude=");
    builder.append(this.latitude);
    builder.append(", longitude=");
    builder.append(this.longitude);
    builder.append(", elevation=");
    builder.append(this.elevation);
    builder.append(", speed=");
    builder.append(this.speed);
    builder.append(", track=");
    builder.append(this.track);
    builder.append(", magneticTrack=");
    builder.append(this.magneticTrack);
    builder.append(", magneticVariation=");
    builder.append(this.magneticVariation);
    builder.append(", separation=");
    builder.append(this.separation);
    builder.append(", dgpsAge=");
    builder.append(this.dgpsAge);
    builder.append(", dgpsSid=");
    builder.append(this.dgpsSid);
    builder.append(", satellites=");
    builder.append(this.satellites);
    builder.append("]");
    return builder.toString();
  }
}