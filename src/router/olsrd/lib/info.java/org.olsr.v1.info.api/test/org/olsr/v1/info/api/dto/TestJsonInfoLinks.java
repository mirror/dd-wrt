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
public class TestJsonInfoLinks {
  private JsonInfoLinks impl = null;

  @Before
  public void setUp() {
    this.impl = new JsonInfoLinks();
  }

  @After
  public void tearDown() {
    this.impl = null;
  }

  @Test(timeout = 8000)
  public void testGettersAndSetters() {
    final JsonInfoLinksEntry entry = new JsonInfoLinksEntry();
    entry.setCurrentLinkStatus("currentLinkStatus");
    final Set<JsonInfoLinksEntry> links = new TreeSet<>();
    links.add(entry);

    /* initial */
    assertThat(this.impl.getLinks(), notNullValue());
    assertThat(this.impl.getLinks(), not(equalTo(links)));

    /* set */
    this.impl.setLinks(links);

    /* get */
    assertThat(this.impl.getLinks(), equalTo(links));

    this.impl.setLinks(null);
    final Set<JsonInfoLinksEntry> exp = new TreeSet<>();
    assertThat(this.impl.getLinks(), equalTo(exp));
  }

  @Test(timeout = 8000)
  public void testEquals() {
    final JsonInfoLinksEntry entry = new JsonInfoLinksEntry();
    entry.setValidityTime(123);
    final List<JsonInfoLinksEntry> links = new LinkedList<>();
    links.add(entry);

    boolean r;
    final Object otherNull = null;
    final Object otherOtherClass = new Object();

    final JsonInfoLinksEntry otherNotSame1Entry = new JsonInfoLinksEntry();
    otherNotSame1Entry.setValidityTime(432);
    final Set<JsonInfoLinksEntry> otherNotSame1hnas = new TreeSet<>();
    otherNotSame1hnas.add(otherNotSame1Entry);
    final JsonInfoLinks otherNotSame1 = new JsonInfoLinks();
    otherNotSame1.setLinks(otherNotSame1hnas);

    final JsonInfoLinks otherNotSame2 = new JsonInfoLinks();

    r = this.impl.equals(this.impl);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));

    r = this.impl.equals(otherNull);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherOtherClass);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    this.impl.setLinks(null);
    r = this.impl.equals(otherNotSame1);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.FALSE));

    otherNotSame2.setLinks(null);
    r = this.impl.equals(otherNotSame2);
    assertThat(Boolean.valueOf(r), equalTo(Boolean.TRUE));
  }

  @Test(timeout = 8000)
  public void testHashCode() {
    int r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(1742810335)));

    final JsonInfoLinksEntry entry = new JsonInfoLinksEntry();
    entry.setCurrentLinkStatus("currentLinkStatus");
    final Set<JsonInfoLinksEntry> links = new TreeSet<>();
    links.add(entry);

    this.impl.setLinks(links);

    r = this.impl.hashCode();
    assertThat(Integer.valueOf(r), equalTo(Integer.valueOf(913836921)));
  }

  @Test(timeout = 8000)
  public void testToString() {
    final String r = this.impl.toString();
    assertThat(r, notNullValue());
  }

  @Test(timeout = 8000)
  public void testJSON() throws IOException {
    final String fn = "doc/examples/links.json";

    final String output = Helpers.readFile(new File(fn));
    final JsonInfoLinks links = Helpers.objectMapper.readValue(output, JsonInfoLinks.class);
    assertThat(links, notNullValue());
  }
}