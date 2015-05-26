#include "gestionfichier.h"
using namespace std;

GestionFichier::GestionFichier()
{
}

void GestionFichier::ouvrir(QString empl,Cube c){

    QFile file(empl);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text)){
        qDebug()<<"le fichier n'est pas ouvert";
       return;
    }

    QTextStream flux(&file);
    QString readLine;
    QString newLine;
    for (int k=0;k<2;k++){
        readLine= flux.readLine()+'\n';
        newLine+=readLine;
    }
    int nplan=0;// numéro du plan
    int nligne=0; //numéro de la ligne

   for(int cntP=0;cntP<9;cntP++){
       for(int cntL=0;cntL<9;cntL++){
           readLine= flux.readLine()+'\n';
           newLine+=getLinePlan(nplan,nligne,c)+'\n';
           nligne++; //passer à la ligne suivante
       }
       for (int i=0;i<2;i++){//recopier la ligne vide et la ligne #2 par exemple
        readLine= flux.readLine()+'\n';
        newLine+=readLine;
       }
        nligne=0;
        nplan++;// passer au plan suivant
   }
   file.close();
   modifierFichier(newLine,empl);
}

QString GestionFichier::getLinePlan(int nplan,int nligne,Cube c){
    QString str="";
    QString etat;
    for(int i=0;i<9;i++){
       Led l=c.getList1()->value(nplan).getLed(nligne,i);
       int e=l.getEtat() ;
       etat=QString::number(e);
       str+=etat;
    }
    return str;
}

void GestionFichier::modifierFichier(QString newLine, QString empl){
    QFile file(empl);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"le fichier n'est pas ouvert";
       return;
    }
    QTextStream out(&file);
    out << newLine;
    file.flush();
    file.close();
}

