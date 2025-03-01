/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

import java.sql.*;
     
public class ForeignKeysCoffees  {

	public static void main(String args[]) {
            
            String url = "jdbc:sqlite:/path_to_db";
            Connection con;
            String createString = "create table COFFEESFK " +
                "(COF_NAME varchar(32) NOT NULL, " + "SUP_ID int, " +
		"PRICE float, " + "SALES int, " + "TOTAL int, " +
                "primary key(COF_NAME), " +
                "foreign key(SUP_ID) references SUPPLIERSPK(SUP_ID))";
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
                stmt.execute("PRAGMA foreign_keys = ON;");
                stmt.executeUpdate("drop table if exists COFFEESFK");
                stmt.executeUpdate(createString);
	
                DatabaseMetaData dbmd = con.getMetaData();	
                ResultSet rs = dbmd.getImportedKeys(null, null, "COFFEESFK");
                while (rs.next()) {
                    System.out.println("primary key table name:  " +
                        rs.getString("PKTABLE_NAME"));
                    System.out.println("primary key column name:  " +
                        rs.getString("PKCOLUMN_NAME"));
                    System.out.println("foreign key table name:  " +
                        rs.getString("FKTABLE_NAME"));
                    System.out.println("foreign key column name:  " +
                        rs.getString("FKCOLUMN_NAME"));
                }

                stmt.executeUpdate("insert into COFFEESFK " +
		     "values('Colombian', 00101, 7.99, 0, 0)");
		    
                stmt.executeUpdate("insert into COFFEESFK " +
		     "values('French_Roast', 00049, 8.99, 0, 0)");
		    
                stmt.executeUpdate("insert into COFFEESFK " +
		     "values('Espresso', 00150, 9.99, 0, 0)");
		    
                stmt.executeUpdate("insert into COFFEESFK " +
		     "values('Colombian_Decaf', 00101, 8.99, 0, 0)");
		    
                stmt.executeUpdate("insert into COFFEESFK " +
		     "values('French_Roast_Decaf', 00150, 9.99, 0, 0)");
		    
                rs = stmt.executeQuery("select * from COFFEESFK");
		    
                System.out.println("select * from COFFEESFK:");
                while (rs.next()) {
                    String name = rs.getString("COF_NAME");
                    int id = rs.getInt("SUP_ID");
                    float price = rs.getFloat("PRICE");
                    int sales = rs.getInt("Sales");
                    int total = rs.getInt("Total");
                    System.out.println(name + ", " + id + ", " +
                        price + ", " + sales + ", " + total);
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

