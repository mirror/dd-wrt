package org.olsr.v1.info.api.dto;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNot.not;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

@SuppressWarnings("static-method")
public class TestJsonInfoHna {
  private JsonInfoHna impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoHna();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    final JsonInfoHnaEntry entry = new JsonInfoHnaEntry();
    entry.setDestinationPrefixLength(1);
    final Set<JsonInfoHnaEntry> hnas = new TreeSet<>();
    hnas.add(entry);

    /* initial */
    assertThat(this.impl.getHna(), notNullValue());
    assertThat(this.impl.getHna(), not(equalTo(hnas)));

    /* set */
    this.impl.setHna(hnas);

    /* get */
    assertThat(this.impl.getHna(), equalTo(hnas));

    this.impl.setHna(null);
    final Set<JsonInfoHnaEntry> exp = new TreeSet<>();
    assertThat(this.impl.getHna(), equalTo(exp));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final JsonInfoHnaEntry entry = new JsonInfoHnaEntry();
    entry.setValidityTime(123);
    final List<JsonInfoHnaEntry> hnas = new LinkedList<>();
    hnas.add(entry);

    boolean r;
    final Object otherNull = null;
    final Object otherOtherClass = new Object();

    final JsonInfoHnaEntry otherNotSame1Entry = new JsonInfoHnaEntry();
    otherNotSame1Entry.setValidityTime(432);
    final Set<JsonInfoHnaEntry> otherNotSame1hnas = new TreeSet<>();
    otherNotSame1hnas.add(otherNotSame1Entry);
    final JsonInfoHna otherNotSame1 = new JsonInfoHna();
    otherNotSame1.setHna(otherNotSame1hnas);

    final JsonInfoHna otherNotSame2 = new JsonInfoHna();

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherOtherClass);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setHna(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame2.setHna(null);
    r = this.impl.equals(otherNotSame2);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    this.impl.setHna(otherNotSame1.getHna());
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810335)));

    final JsonInfoHnaEntry entry = new JsonInfoHnaEntry();
    entry.setDestinationPrefixLength(1);
    final Set<JsonInfoHnaEntry> hnas = new TreeSet<>();
    hnas.add(entry);

    this.impl.setHna(hnas);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1743733887)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/hna.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoHna hna = Helpers.objectMapper.readValue(output, JsonInfoHna.class);
    assertThat(hna, notNullValue());
  }
}