/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package uimodel.am;

import oracle.jbo.ApplicationModule;
import oracle.jbo.Key;
import oracle.jbo.Row;
import oracle.jbo.RowMatch;
import oracle.jbo.ViewCriteria;
import oracle.jbo.ViewCriteriaRow;
import oracle.jbo.ViewObject;
import oracle.jbo.client.Configuration;

public class TestClient {
    public static void main(String[] args) {
        
        String amDef = "uimodel.am.AppModule";
        String config = "AppModuleLocal";
        ApplicationModule am =
            Configuration.createRootApplicationModule(amDef, config);

        ViewObject cofVO = am.findViewObject("Coffees");
        ViewObject supVO = am.findViewObject("Suppliers");
        cofVO.executeQuery();
        supVO.executeQuery();
        showRows(cofVO, "Initial database results - Coffees");
        showRows(supVO, "Initial database results - Suppliers");
        
        // Test sorting rows.
        System.out.println("\n=== " + " Test sorting rows " + " ===\n");
        cofVO.setSortBy("Price desc");
        cofVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_VIEW_ROWS);
        cofVO.executeQuery();
        showRows(cofVO, "Sorting Coffees by Price desc");
        
        supVO.setSortBy("Zip asc");
        supVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_VIEW_ROWS);
        supVO.executeQuery();
        showRows(supVO, "Sorting Suppliers by Zip asc");
        System.out.println("\n=== " + " End " + " ===\n");
        
        // Test matching rows.
        System.out.println("\n=== " + " Test matching rows " + " ===\n");
        cofVO.setSortBy("");
        supVO.setSortBy("");
        
        RowMatch rm = new RowMatch("CofName like 'C%'");
        cofVO.setRowMatch(rm);
        cofVO.executeQuery();
        showRows(cofVO, "Coffee Name begins with 'C'");
        
        rm = new RowMatch("Zip like '95%'");
        supVO.setRowMatch(rm);
        supVO.executeQuery();
        showRows(supVO, "Supplier Zip begins with '95'");
        System.out.println("\n=== " + " End " + " ===\n");

        // Test filtering rows by view criteria.
        System.out.println("\n=== " +
            " Test filtering rows by view criteria " + " ===\n");
        cofVO.setRowMatch(null);
        
        ViewCriteria vc = cofVO.createViewCriteria();
        ViewCriteriaRow vcr = vc.createViewCriteriaRow();
        vcr.setAttribute("SupId", "= 150");
        vc.add(vcr);
        cofVO.applyViewCriteria(vc);
        vc.setCriteriaMode(ViewCriteria.CRITERIA_MODE_CACHE);
        cofVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_ENTITY_ROWS);
        cofVO.executeQuery();
        showRows(cofVO, "Supplier ID = 150");
        System.out.println("\n=== " + " End " + " ===\n");
        
        // Test inserting a row with the foreign key.
        System.out.println("\n=== " +
            " Test insert with a foreign key " + " ===\n");
        cofVO.applyViewCriteria(null);
        
        cofVO.setSortBy("SupId");
        cofVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_VIEW_ROWS);
        cofVO.executeQuery();
        showRows(cofVO, "Before insert and clear cache, query over VO cache");
        cofVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_ENTITY_ROWS);
        cofVO.executeQuery();
        showRows(cofVO, "Before insert and clear cache, query over EO cache");
        
        cofVO.setAssociationConsistent(true);
        Row newCof = cofVO.createRow();
        newCof.setAttribute("CofName", "Java_Chips_Mocha");
        newCof.setAttribute("Price", "5.99");
        newCof.setAttribute("Sales", "5");
        newCof.setAttribute("Total", "9");
        try {
            newCof.setAttribute("SupId", "10");
            System.out.println("Setting an invalid foreign key should fail.");
            return;
        } catch (Exception e) {
        }       
        newCof.setAttribute("SupId", "101");
        cofVO.insertRow(newCof);
        cofVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_VIEW_ROWS);
        cofVO.clearCache();
        showRows(cofVO, "After insert and clear cache, query over VO cache");
        
        cofVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_ENTITY_ROWS);
        cofVO.executeQuery();
        showRows(cofVO, "After insert and clear cache, query over EO cache");
        System.out.println("\n=== " + " End " + " ===\n");
        
        // Test updating a row.
        System.out.println("\n=== " + " Test updating a row " + " ===\n");
        showRows(cofVO, "Before update");

        newCof.setAttribute("Price", "15.99");
        cofVO.clearCache();
        showRows(cofVO, "After update");
        System.out.println("\n=== " + " End " + " ===\n");
        
        // Test deleting a row in SUPPLIERSPK and all rows referencing
        // the same SupId in COFFEESFK will be deleted automactially.
        System.out.println("\n=== " + " Test deleting rows " + " ===\n");
        supVO.setRowMatch(null);
        supVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_ENTITY_ROWS);
        supVO.executeQuery();
        showRows(supVO, "Before delete in SUPPLIERS - Suppliers");
        showRows(cofVO, "Before delete in SUPPLIERS - Coffees");

        supVO.setAssociationConsistent(true);
        Object[] keyValues = new Object[6];
        keyValues[0] = new Integer(49);
        for (int i = 1; i < keyValues.length; i++)
            keyValues[i] = null;
        Row[] rows = supVO.findByKey(new Key(keyValues), 1);
        rows[0].remove();
        supVO.setQueryMode(ViewObject.QUERY_MODE_SCAN_ENTITY_ROWS);
        supVO.executeQuery();
        showRows(supVO, "After delete in SUPPLIERS - Suppliers");
        showRows(cofVO, "After delete in SUPPLIERS - Coffees");
        System.out.println("\n=== " + " End " + " ===\n");
        
        Configuration.releaseRootApplicationModule(am, true);    
    }
    
    private static void showRows(ViewObject vo, String msg) {
        System.out.println("\n--- " + msg + " ---\n");
        boolean prtCof = false;
        if (vo.getFullName().contains("Coffees") == true)
            prtCof = true;
        String prtStr;
        vo.reset();      
        while (vo.hasNext()) {
            Row r = vo.next();
            if (prtCof == true) {
                String name = (String)r.getAttribute("CofName");
                Integer id = (Integer)r.getAttribute("SupId");
                Float price = (Float)r.getAttribute("Price");
                Integer sales = (Integer)r.getAttribute("Sales");
                Integer total = (Integer)r.getAttribute("Total");
                prtStr = name + ", " + id + ", " +
                    price + ", " + sales + ", " + total;
            } else {
                Integer id = (Integer)r.getAttribute("SupId");
                String name = (String)r.getAttribute("SupName");
                String street = (String)r.getAttribute("Street");
                String city = (String)r.getAttribute("City");
                String state = (String)r.getAttribute("State");
                String zip = (String)r.getAttribute("Zip");
                prtStr = id + ", " + name + ", " +
                    street + ", " + city + ", " + state + ", " + zip;
            }
            System.out.println(prtStr);
        }
    }
}
