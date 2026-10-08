// Copyright AStarship.

package leukocytewars;

import javax.swing.*;

public class TheLeukocyteWars extends JApplet
{
    public TheLeukocyteWars ()
    {
        add (new LeukocyteWar ());
    }
    
    public static void main (String[] args)
    {
        JFrame window = new JFrame ("The Leukocyte Wars");
        window.setDefaultCloseOperation (JFrame.EXIT_ON_CLOSE);
        
        JScrollPane scrollableArea = new JScrollPane (new LeukocyteWar ());
        
        window.setContentPane (scrollableArea);
        
        window.pack ();
        window.setVisible (true);
    }
}
