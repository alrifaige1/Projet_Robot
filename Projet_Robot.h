//  Copyright (c) 2019 Antoine Tran Tan

//



#include "IHM.h" // Ne jamais oublier les bibliotheques

#include "NBoard.h"

#include <PwmOut.h>



//définition des variables



PwmOut PWM0(PB_5);             //roue droite

PwmOut PWM1(PB_4);             //roue gauche

double cpt0, cpt1, cpt2, cpt3; //capteurs

double valcpt;

Timer timer;



void automate(void); //déclaration de l'automate



int main()

{



    PWM0.period_us(50); //réglage de la PWM pour chaque roue

    PWM1.period_us(50);



    while (true) 

    {

        wait_ms(5);

        automate();

    }

}



void automate(void) //définition de l'automate

{

    bool jack;               //création des variables pour gérer le jack, le bouton d'arrêt, les moteurs et le variateur de vitesse (potentiomètre)

    unsigned char  bouton;

    static double MotG, MotD;

    static float pot;

    double codeur = 0;



    BusSelectMux = Vpot;

    wait_us(1);

    pot = AnaIn;

    BusSelectMux = Ana0; //choix des capteurs à la sortie du mux

    wait_us(1);

    cpt0 = AnaIn;

    BusSelectMux = Ana1;

    wait_us(1);

    cpt1 = AnaIn;

    BusSelectMux = Ana2;

    wait_us(1);

    cpt2 = AnaIn;



    BusSelectMux = Ana3;

    wait_us(1);

    cpt3 = AnaIn;

    Bus8Led = 0x00;



    PWM1 = MotG; //associations des variables

    PWM0 = MotD;

    jack = Num0;

    bouton = !Num1;



    //le codeur permettait de régler les ajustememnts, il a été retiré du code car il n'est plus utile.

    codeur = ihm.COD_read();

    if (codeur > 127)

    {

        codeur = codeur - 256;

    }

    codeur = codeur / 10;



    pot = pot * 10; //potentiomètre entre 0 et 4 et affichage de sa valeur sur le LCD

    ihm.LCD_gotoxy(0, 0);

    ihm.LCD_printf("%.2f  %.4f", pot, codeur);



    typedef enum //définition des états de l'automate

    {

        depart,

        arret,

        roule,

        raccourci,

        virageg,

        attente

    } type_etat;

    static type_etat etat = depart;



    switch (etat)

    {



        //état départ

    case depart:



        MotD = 0.0; // les deux moteurs ne tournent pas

        MotG = 0.0;



        if (jack == 0 ) //enlever le jack fait avancer le robot (envoie dans l'état roule)



        {

            MotG = 0.108 * pot;

            MotD = 0.1 * pot;

            etat = roule;

        }

        break;



        //état arrêt

    case arret:



        MotD = 0.0; // les deux moteurs ne tournent pas

        MotG = 0.0;



        if (jack == 1)

        {

            etat = depart; //envoie dans l'état départ si le jack est rebranché

        }

        break;



        //état raccourci

    case raccourci:

        ihm.LCD_gotoxy(1, 0);

        ihm.LCD_printf("etat raccourci");



        if (jack == 1) //si le jack est branché, retour à l'état départ

        {

            etat = depart;

        }

        else if (jack == 0 && bouton == 1) // si le jack est débranché ET bouton de fin de course enfoncé, arrêt du robot

        {

            etat = arret;

        }



        wait_ms(300);

        MotG = 0.0;

        MotD = 0.1;

        wait_ms(500);

        timer.reset();

        timer.stop();

        etat = roule;

        break;



    //etat virage gauche

    case virageg:

        ihm.LCD_gotoxy(1, 0);

        ihm.LCD_printf("etat raccourci");

        if (jack == 1) //si le jack est branché, retour à l'état départ

        {

            etat = depart;

        }

        else if (jack == 0 && bouton == 1) // si le jack est débranché ET bouton de fin de course enfoncé, arrêt du robot

        {

            etat = arret;

        }

        else

        {

            MotG = 0.15512;

            MotD = 0.35;

            ihm.LCD_gotoxy(1, 0);

            ihm.LCD_printf("G:%5.3f D:%5.3f", MotG, MotD);

            Bus8Led = 0x08; //les leds permettent de voir quel capteur est actif

            timer.reset(); //le timer sert pour le raccourci (n'est pas utilisé ici)

            timer.stop();

            etat = roule;

        }

        break;



        //état d'attente pour le choix entre le virage à gauche et le raccourci

    case attente:

        if (cpt0 > 0.5 && timer.read_ms() < 500)

        {

            etat = virageg; //emmène dans l'état raccourci normalement mais celui-ci ne fonctionne pas

        }

        else

        {

            etat = virageg;

        }

    

        break;



        //état roule

    case roule:

        ihm.LCD_gotoxy(0, 0);

        ihm.LCD_printf("etat roule");



        if (jack == 1) //si le jack est branché, retour à l'état départ

        {

            etat = depart;

        }

        else if (jack == 0 && bouton == 1) // si le jack est débranché ET bouton de fin de course enfoncé, arrêt du robot

        {

            etat = arret;

        }



        //ajustement en ligne droite

        else if (cpt1 < 0.5 && cpt2 > 0.5 && cpt0 > 0.5)

        {

            valcpt = -0.0001;



            MotG = MotG + (0.2 * valcpt) * 3.5;

            MotD = MotD - (0.2 * valcpt) * 3.5;

            ihm.LCD_gotoxy(1, 0);

            ihm.LCD_printf("G:%5.3f D:%5.3f", MotG, MotD);

            Bus8Led = 0x02; //les leds permettent de voir quel capteur est actif



            //borne d'ajustement pour empêcher les "zig zags"

            if (MotG < 0.109692 * pot)

            {

                MotG = 0.109692 * pot;

            }

            else if (MotD > 0.102 * pot)

            {

                MotD = 0.102 * pot;

            }

        }



        else if (cpt1 > 0.5 && cpt2 < 0.5 && cpt3 > 0.5)

        {

            valcpt = 0.0001;



            MotG = MotG + (0.2 * valcpt) * 3.5;

            MotD = MotD - (0.2 * valcpt) * 3.5;

            ihm.LCD_gotoxy(1, 0);

            ihm.LCD_printf("G:%5.3f D:%5.3f", MotG, MotD);

            Bus8Led = 0x04;  //les leds permettent de voir quel capteur est actif



             //borne d'ajustement pour empêcher les "zig zags"

            if (MotG > 0.11908 * pot)

            {

                MotG = 0.113016 * pot;

            }

            else if (MotD < 0.099 * pot)

            {

                MotD = 0.099 * pot;

            }

        }



        //grands virages ou raccourci



        //envoie dans l'état attente si capteur extrême gauche capte quelque chose (virage à gauche ou prise de raccourci)

        else if (cpt0 < 0.5)

        {

            timer.start();

            timer.reset();

            etat = attente;

        }



        //virage à droite

        else if (cpt3 < 0.5 && cpt1 > 0.5)

        {



            MotG = 0.3878;

            MotD = 0.14;



            ihm.LCD_gotoxy(1, 0);

            ihm.LCD_printf("G:%5.3f D:%5.3f", MotG, MotD);

            Bus8Led = 0x01; //les leds permettent de voir quel capteur est actif

        }

        break;

    }

}

