/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

import java.sql.*;
     
public class PrimaryKeysSuppliers  {

	public static void main(String args[]) {
            
            String url = "jdbc:sqlite:/path_to_db";
            Connection con;
            String createString = "create table SUPPLIERSPK " +
                "(SUP_ID INTEGER NOT NULL, " + "SUP_NAME VARCHAR(40), " +
                "STREET VARCHAR(40), " + "CITY VARCHAR(20), " +
                "STATE CHAR(2), " + "ZIP CHAR(5), " +
		"primary key(SUP_ID))";
            Statement stmt;
	
            try {
                Class.forName("SQLite.JDBCDriver");
            } catch(java.lang.ClassNotFoundException e) {
                System.err.print("ClassNotFoundException: "); 
                System.err.println(e.getMessage());
            }

            try {
                con = DriverManager.getConnection(url, 
		    "myLogin", "myPassword");
			
                stmt  = con.createStatement();
                stmt.executeUpdate("drop table if exists SUPPLIERSPK");
                stmt.executeUpdate(createString);
                    
                stmt.executeUpdate("insert into SUPPLIERSPK " +
                    "values(49, 'Superior Coffee', '1 Party Place', " +
                    "'Mendocino', 'CA', '95460')");
		    
                stmt.executeUpdate("insert into SUPPLIERSPK " +
                    "values(101, 'Acme, Inc.', '99 Market Street', " +
                    "'Groundsville', 'CA', '95199')");
		    
                stmt.executeUpdate("insert into SUPPLIERSPK " +
                    "values(150, 'The High Ground', '100 Coffee Lane', " +
                        "'Meadows', 'CA', '93966')");
		    
                ResultSet rs = stmt.executeQuery("select * from SUPPLIERSPK");
		    
                System.out.println("select * from SUPPLIERSPK:");
                while (rs.next()) {
                    int id = rs.getInt("SUP_ID");
                    String name = rs.getString("SUP_NAME");
                    String street = rs.getString("STREET");
                    String city = rs.getString("CITY");
                    String state = rs.getString("STATE");
                    String zip = rs.getString("ZIP");
                    System.out.println(id + ", " + name + ", " +
                        street + ", " + city + ", " + state + ", " + zip);
                }

                rs.close();
                stmt.close();
                con.close();
	
            } catch(SQLException ex) {
                System.err.print("SQLException: ");
                System.err.println(ex.getMessage());
            }
	}
}

