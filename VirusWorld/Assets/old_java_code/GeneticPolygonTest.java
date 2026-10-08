// Copyright AStarship.

package leukocytewars;

import javax.swing.*;

public class GeneticPolygonTest extends JApplet
{   
    public GeneticPolygonTest ()
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
