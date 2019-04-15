package org.olsr.v1.info.api.util;

import static org.hamcrest.core.IsEqual.equalTo;
import static org.hamcrest.core.IsNull.notNullValue;
import static org.junit.Assert.assertThat;

import org.junit.Test;

@SuppressWarnings("static-method")
public class TestCompareUtils {
  @Test(timeout = 8000)
  public void testInstance() {
    final CompareUtils cu = new CompareUtils();
    assertThat(cu.toString(), notNullValue());
  }

  @Test(timeout = 8000)
  public void testClip() {
    assertThat(Integer.valueOf(CompareUtils.clip(-2)), equalTo(Integer.valueOf(-1)));
    assertThat(Integer.valueOf(CompareUtils.clip(-1)), equalTo(Integer.valueOf(-1)));
    assertThat(Integer.valueOf(CompareUtils.clip(0)), equalTo(Integer.valueOf(0)));
    assertThat(Integer.valueOf(CompareUtils.clip(1)), equalTo(Integer.valueOf(1)));
    assertThat(Integer.valueOf(CompareUtils.clip(2)), equalTo(Integer.valueOf(1)));
  }
}