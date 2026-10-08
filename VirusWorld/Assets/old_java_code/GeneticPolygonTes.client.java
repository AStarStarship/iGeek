// Copyright AStarship.

package leukocytewars;

import javax.swing.*;

public class GeneticPolygonTestApp extends JApplet
{   
    public GeneticPolygonTestApp ()
    {
        add (new GeneticPolygonTestPanel ());
    }
    
    public static void main (String[] args)
    {
        JFrame window = new JFrame ("GeneticPolygon Test App");
        window.setDefaultCloseOperation (JFrame.EXIT_ON_CLOSE);

        window.setContentPane (new GeneticPolygonTestPanel ());
        
        window.pack ();
        window.setVisible (true);
    }
}
