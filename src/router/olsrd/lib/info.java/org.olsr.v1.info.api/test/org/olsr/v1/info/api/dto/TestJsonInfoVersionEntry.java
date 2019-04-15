package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TestJsonInfoVersionEntry {
  private JsonInfoVersionEntry impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoVersionEntry();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    /* initial */
    assertThat(this.impl.getVersion(), equalTo(""));
    assertThat(this.impl.getGitDescriptor(), equalTo(""));
    assertThat(this.impl.getGitSha(), equalTo(""));
    assertThat(this.impl.getReleaseVersion(), equalTo(""));
    assertThat(this.impl.getSourceHash(), equalTo(""));

    /* set */
    this.impl.setVersion("version");
    this.impl.setGitDescriptor("gitDescriptor");
    this.impl.setGitSha("gitSha");
    this.impl.setReleaseVersion("releaseVersion");
    this.impl.setSourceHash("sourceHash");

    /* get */
    assertThat(this.impl.getVersion(), equalTo("version"));
    assertThat(this.impl.getGitDescriptor(), equalTo("gitDescriptor"));
    assertThat(this.impl.getGitSha(), equalTo("gitSha"));
    assertThat(this.impl.getReleaseVersion(), equalTo("releaseVersion"));
    assertThat(this.impl.getSourceHash(), equalTo("sourceHash"));
  }

  @Test(timeout = 8000)
  public void testCompareTo() {
    int r;
    final JsonInfoVersionEntry other = new JsonInfoVersionEntry();

    final String version1 = "version1";
    final String version2 = "version2";
    final String host1 = "host1";
    final String host2 = "host2";

    /* null */

    r = this.impl.compareTo(null);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    /* version */

    final String versionOrg = this.impl.getVersion();

    this.impl.setVersion(null);
    other.setVersion(version2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setVersion(version2);
    other.setVersion(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setVersion(version1);
    other.setVersion(version2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setVersion(version1);
    other.setVersion(version1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setVersion(versionOrg);
    other.setVersion(versionOrg);

    /* gitDescriptor */

    final String gitDescriptorOrg = this.impl.getGitDescriptor();

    this.impl.setGitDescriptor(null);
    other.setGitDescriptor(host2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGitDescriptor(host2);
    other.setGitDescriptor(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGitDescriptor(host1);
    other.setGitDescriptor(host2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGitDescriptor(host1);
    other.setGitDescriptor(host1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGitDescriptor(gitDescriptorOrg);
    other.setGitDescriptor(gitDescriptorOrg);

    /* gitSha */

    final String gitShaOrg = this.impl.getGitSha();

    this.impl.setGitSha(null);
    other.setGitSha(host2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGitSha(host2);
    other.setGitSha(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setGitSha(host1);
    other.setGitSha(host2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setGitSha(host1);
    other.setGitSha(host1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setGitSha(gitShaOrg);
    other.setGitSha(gitShaOrg);

    /* releaseVersion */

    final String releaseVersionOrg = this.impl.getReleaseVersion();

    this.impl.setReleaseVersion(null);
    other.setReleaseVersion(host2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setReleaseVersion(host2);
    other.setReleaseVersion(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setReleaseVersion(host1);
    other.setReleaseVersion(host2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setReleaseVersion(host1);
    other.setReleaseVersion(host1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setReleaseVersion(releaseVersionOrg);
    other.setReleaseVersion(releaseVersionOrg);

    /* sourceHash */

    final String sourceHashOrg = this.impl.getSourceHash();

    this.impl.setSourceHash(null);
    other.setSourceHash(host2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSourceHash(host2);
    other.setSourceHash(null);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1)));

    this.impl.setSourceHash(host1);
    other.setSourceHash(host2);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(-1)));

    this.impl.setSourceHash(host1);
    other.setSourceHash(host1);
    r = this.impl.compareTo(other);
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(0)));

    this.impl.setSourceHash(sourceHashOrg);
    other.setSourceHash(sourceHashOrg);
  }

  @Test(timeout = 8000)
  public void testEquals() {
    boolean r;
    JsonInfoVersionEntry other;

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other = null;
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    final Object otherObj = new Object();
    r = this.impl.equals(otherObj);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    other = new JsonInfoVersionEntry();
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    other.setVersion("version");
    r = this.impl.equals(other);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(28629151)));

    this.impl.setVersion("version");
    this.impl.setGitDescriptor("gitDescriptor");
    this.impl.setGitSha("gitSha");
    this.impl.setReleaseVersion("releaseVersion");
    this.impl.setSourceHash("sourceHash");

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1960702216)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }
}