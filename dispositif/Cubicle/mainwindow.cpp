#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTreeWidgetItem"
#include"QInputDialog"
#include "nouveaumotif.h"
#include "QMenu"
#include "QPoint"
#include "QMessageBox"
#include "QDirIterator"
#include "QDebug"
#include <strstream>
#include <string>
#include <sstream>
#include <iostream>
#include "gestionfichier.h"
#include "cube.h"
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>




using namespace std;

//MainWindow *MainWindow::_instance = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);


    ui->actionCopy->setDisabled(true);
    ui->actionDelete_pattern->setDisabled(true);
    ui->actionNew_Group->setDisabled(true);
    ui->actionNew_Pattern->setDisabled(true);
    ui->actionPaste_pattern->setDisabled(true);
    ui->actionSave->setDisabled(true);
    ui->actionCut_pattern->setDisabled(true);


    //désactiver la sélection des plans
    ui->plane1->setDisabled(true);
     ui->plane2->setDisabled(true);
      ui->plane3->setDisabled(true);
       ui->plane4->setDisabled(true);
         ui->plane5->setDisabled(true);
          ui->plane6->setDisabled(true);
           ui->plane7->setDisabled(true);
            ui->plane8->setDisabled(true);
            ui->plane9->setDisabled(true);
    connect(ui->actionNew_project,SIGNAL(triggered(bool)),this,SLOT(new_project()));
    connect(ui->actionOpen_directory,SIGNAL(triggered(bool)),this,SLOT(ouvrir_explorer()));
    connect(ui->actionCopy,SIGNAL(triggered(bool)),this,SLOT(copier()));
    connect(ui->actionPaste_pattern,SIGNAL(triggered(bool)),this,SLOT(coller()));
    connect(ui->actionNew_Pattern,SIGNAL(triggered(bool)),this,SLOT(ajouter_motif()));
    connect(ui->actionQuit,SIGNAL(triggered(bool)),this,SLOT(controlQuit()));
    connect(ui->actionDelete_pattern,SIGNAL(triggered(bool)),this,SLOT(controlDelete()));
    connect(ui->actionSave,SIGNAL(triggered(bool)),this,SLOT(controlSave()));
    connect(ui->treeView,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(doubleClick()));
    connect(ui->actionCut_pattern,SIGNAL(triggered(bool)),this,SLOT(couper()));
    connect(ui->actionSave_as,SIGNAL(triggered(bool)),this,SLOT(controlSaveAs()));
   // connect(ui->treeView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(controlRename()));

    this->setWindowTitle("Cubicle");
    deletePlanLed(0);
    desactivePlan(0);
    connexion();
    dirOpen=false;




}

//ouvre le répertoire de travail



void MainWindow::ouvrir_explorer(){
    namedir=QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                   "/home"
                                               );
  if (namedir=="") {qDebug()<<namedir;
      return;}

  QDir dir(namedir);
  QStringList nameFilter;
  nameFilter<<"*.txt";
  QFileInfoList list=dir.entryInfoList(nameFilter,QDir::Files);
  QFileInfoList list2=dir.entryInfoList(QDir::Dirs);
  //on ne doit pas charger le dossier d'un groupe de motif mais plutot le repertoire des groupes de motif
  if ((!list.isEmpty()) and (!list2.isEmpty())) {
       QMessageBox::information(this,tr("warning"),"cannot open this directory, please choose the source directory");
       qDebug()<<"impossible";
       return;
  }
  tree();
  dirOpen=true;
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event){
    if(dirOpen){
        contextMenu = new QMenu(ui->treeView);
        QModelIndex index=ui->treeView->currentIndex();
        if (model->fileInfo(index).isDir()) {
             QString s =model->fileInfo(index).absoluteFilePath();
             if(index.isValid()){
                  ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
                  QString dir=model->fileInfo(index).absolutePath();
                  QString nameGroup=model->fileInfo(index).baseName();
                  if((dir+'/'+nameGroup)!=namedir){
                          insertMotif = contextMenu->addAction("new pattern");
                          connect(insertMotif,SIGNAL(triggered(bool)),this, SLOT(ajouter_motif()));
                          QAction * paste;
                          paste = contextMenu->addAction("paste pattern");
                          connect(paste,SIGNAL(triggered(bool)),this, SLOT(coller()));
                  }
             }
    }

        if (model->fileInfo(index).isFile()){
              QString sp =model->fileInfo(index).absoluteFilePath();
              if(index.isValid()){
                   ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
                   QAction * save;
                   save = contextMenu->addAction("save");
                   connect(save,SIGNAL(triggered(bool)),this, SLOT(controlSave()));
                   QAction * copy;
                   copy = contextMenu->addAction("copy pattern");
                   connect(copy,SIGNAL(triggered(bool)),this, SLOT(copier()));
                   QAction *cut = contextMenu->addAction("cut pattern");
                   connect(cut,SIGNAL(triggered(bool)),this, SLOT(couper()));
                   deletePattern = contextMenu->addAction("delete pattern");
                   connect(deletePattern,SIGNAL(triggered(bool)),this, SLOT(controlDelete()));
                }
        }

    contextMenu->exec(QCursor::pos());
    }

}

void MainWindow::couper(){
    copier();
    copierCouper=1;
}

void MainWindow::copier(){
    QModelIndex index=ui->treeView->currentIndex();
    if (model->fileInfo(index).isFile()) {
        dirOrFile=false;
        paste_element=model->fileInfo(index).absoluteFilePath();
        nom_copie=model->fileInfo(index).baseName();
        copierCouper=0;
        qDebug()<<nom_copie;
        qDebug()<<"j'ai copié : "+paste_element;
    }
}
void MainWindow::coller(){
     QString nameGroup;
    if (!dirOrFile){

        QModelIndex index=ui->treeView->currentIndex();
        if (index.isValid()){
            if (model->fileInfo(index).isDir()) {
                QString dir=model->fileInfo(index).absolutePath();
                nameGroup=model->fileInfo(index).baseName();
                if((dir+'/'+nameGroup)!=namedir){
                    QFile file(paste_element);
                     qDebug()<<"je vais coller  :"+dir+"/"+nameGroup+"/"+nom_copie+"_copie.txt";
                    bool valid = file.copy(dir+"/"+nameGroup+"/"+nom_copie+"_copie.txt");
                    if (copierCouper==1){
                        QFile file(paste_element);
                        file.remove();
                        tree();
                    }
                    if (!valid){
                       qDebug()<<"coller impossible";
                   }
                    tree();
                }
          }
        }
    }
}


void MainWindow::tree(){
            model = new QDirModel(this);
            model->setReadOnly(false);
            model->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);

    ui->treeView->setModel(model);
    qDebug() << "le namedir est" + namedir;
    QModelIndex index=model->index(namedir);
     ui->treeView->setRootIndex(index);
    // ui->treeView->setExpanded(new_index,true);

  /*  ui->treeView->selectionModel()->select(new_index,
       QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);*/
    for(int i=1;i<4;i++){
        ui->treeView->hideColumn(i);
    }
    ui->treeView->resizeColumnToContents(0);
    ui->actionNew_Pattern->setEnabled(true);
    ui->actionDelete_pattern->setEnabled(true);
    ui->actionCopy->setDisabled(false);
    ui->actionPaste_pattern->setDisabled(false);
    ui->actionCut_pattern->setDisabled(false);
    ui->actionSave->setDisabled(false);
    ui->actionNew_Group->setDisabled(false);
    reordonneGroup();
}

//créer un nouveau motif
void MainWindow::ajouter_motif(){
    QModelIndex index=ui->treeView->currentIndex();
    if (index.isValid()){

        if (model->fileInfo(index).isDir()) {

            QString dir=model->fileInfo(index).absolutePath();

            QString nameGroup=model->fileInfo(index).baseName();
             if((dir + "/" + nameGroup) !=namedir){
            qDebug()<<"le namedir est "+ namedir;
            qDebug()<<"l'emplacement du dossier est "+dir;
            qDebug()<<"le nom du dossier est "+nameGroup;
            if(nameGroup!="Cubicle"){
                    NouveauMotif m=NouveauMotif("New Pattern",dir+"/"+nameGroup);
                    tree();
                    if(namedir==s+"/workspace"){
                    new_index =model->index(namedir+"/Cubicle/"+ nameGroup );
                 ui->treeView->expand(new_index);
                 ui->treeView->scrollTo(new_index);

                 new_index =model->index(m.getNameFile());
                 ui->treeView->setCurrentIndex(new_index);
                 ui->treeView->selectionModel()->select(new_index,
                        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                  ui->treeView->edit(new_index);

                    }

                else{
                    new_index =model->index(namedir+"/"+ nameGroup );
                 ui->treeView->expand(new_index);
                 ui->treeView->scrollTo(new_index);
                 new_index =model->index(m.getNameFile());
                 ui->treeView->setCurrentIndex(new_index);
                 ui->treeView->selectionModel()->select(new_index,
                        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                    ui->treeView->edit(new_index);

                }


            }
             else {
               QMessageBox::information(this,tr("warning"),"cannot add a pattern, please choose or add a group");
             }

        }
    }

}
}
void MainWindow::new_project(){

    s=QCoreApplication::applicationDirPath();
    model = new QDirModel(this);
     model->setReadOnly(false);
     model->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);

         QModelIndex index=model->index(s);
        model->mkdir(index,"workspace");
        namedir=s+"/workspace";


                  qDebug()<<"je crée cubicle pour la 1ere fois";
             new_index=model->index(namedir);
             model->mkdir(new_index,"Cubicle");

        /*qDebug()<<"je crée cubicle pour la 1ere fois";
             QModelIndex index1=model->index(namedir);
             model->mkdir(index1,"Cubicle");
                    tree();*/


            tree();
}

void MainWindow::on_actionNew_Group_triggered()
{   int m;
    QString indice;
    if (namedir==s+"/workspace"){
    QModelIndex index =model->index(namedir+"/Cubicle",0);
    QString name ="NewGroup";
    QDir dir(namedir+"/Cubicle");
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    dir.setSorting( QDir::Name);
    m=entries.size();
    if (m<10){
        indice = "0"+QString::number(m)+"_";
    }else {
       indice = QString::number(m)+"_";

    }
    qDebug() << "s contient "+ s;
    name = indice + name;
     qDebug() << "s contient "+ name;
    model->mkdir(index,name);
    qDebug()<<"j'ai crée un dossier ds "+namedir;
       new_index =model->index(namedir+"/Cubicle");
     qDebug()<<"le new index est " + namedir+"/Cubicle";
     new_index =model->index(namedir+"/Cubicle");
  ui->treeView->expand(new_index);
  ui->treeView->scrollTo(new_index);

  new_index =model->index(namedir+"/Cubicle/"+name);
  ui->treeView->setCurrentIndex(new_index);
  ui->treeView->selectionModel()->select(new_index,
         QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  ui->treeView->edit(new_index);

}
    else {
        QModelIndex index =model->index(namedir,0);
        QString name ="New Group";
        QDir dir(namedir);
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
        dir.setSorting( QDir::Name);
        m=entries.size();
        if (m<10){
            s = "0"+QString::number(m)+"_";
        }else {
            s = QString::number(m)+"_";

        }
        name = s + name;
        model->mkdir(index,name);
        qDebug()<<"j'ai crée un dossier ds "+namedir;
           new_index =model->index(namedir);
         qDebug()<<"le new index est " + namedir;
         new_index =model->index(namedir);
      ui->treeView->expand(new_index);
      ui->treeView->scrollTo(new_index);

      new_index =model->index(namedir+"/"+name);
      ui->treeView->setCurrentIndex(new_index);
      ui->treeView->selectionModel()->select(new_index,
             QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      ui->treeView->edit(new_index);

    }
}





void MainWindow::controlQuit(){
    int reponse = QMessageBox::question(this, "Quit", " Are you sure you want to quit ?");

        if (reponse == QMessageBox::Yes)
        {
            this->close();
        }
}

void MainWindow::controlDelete(){
    QModelIndex index=ui->treeView->currentIndex();
    if (model->fileInfo(index).isFile()) {
        dirOrFile=false;

         QString name=model->fileInfo(index).absoluteFilePath();
         QFile file(name);
         int reponse = QMessageBox::question(this, "Quit", " Are you sure you want to delete this pattern ?");
           if (reponse == QMessageBox::Yes) {
               file.remove();
               tree();
               this->deletePlanLed(1);
               this->desactivePlan(1);
           }
    }
    else {
        dirOrFile=true;
    }
}

void MainWindow::controlSave(){
    GestionFichier ges;
    ges.ouvrir(this->emplMotif,this->c);
}
void MainWindow::xCopy2 (const QString &sourcePath, const QString &destPath, const QString &name)
{
    static const QStringList filters = QStringList () << "*";
    static const QDir::Filters flags = QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files | QDir::Hidden;

    QString sourceObjectPath = sourcePath+"/"+name;
    QString destObjectPath = destPath +"/"+name;
    QFileInfo fi (sourceObjectPath);

    if (fi.isDir ()) {
        qDebug () << "Créer le répertoire " << destObjectPath;
        QDir destDir(destPath);
        destDir.mkdir(name);

        qDebug () << "Recopier dedans, récursivement, le contenu de" << sourceObjectPath;
        //sourceObjectPath += '/';
        //destObjectPath += '/';
        QDir currentSourceDir (sourceObjectPath);
        const QStringList fileList = currentSourceDir.entryList (filters, flags);
        foreach (const QString &content, fileList) {
            xCopy2 (sourceObjectPath, destObjectPath, content);
        }
    } else {
        qDebug () << "Copier le fichier " << name << "de" << sourcePath << "vers" << destPath;
        QFile::copy (sourceObjectPath, destObjectPath);


    }





}
void MainWindow::controlSaveAs(){
    qDebug()<<"je suis dans controlSaveAs";
    QString destPath=QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                   "/home"
                                               );
    QString originPath=namedir;
    qDebug()<<"l'origine est "+namedir;
    qDebug()<<"la destination est"+destPath;
    xCopy2(originPath,destPath,"Cubicle");
    if (!removeDir(namedir+"/Cubicle")){
        qDebug()<<namedir+"/Cubicle n'est pas supprimé";
    }
    namedir= destPath+"/Cubicle";
    qDebug()<< "le nouveau path est" + namedir;
    this->setWindowTitle("Cubicle["+destPath+"/Cubicle"+"]") ;
    tree();

}

bool MainWindow::removeDir(const QString& dirPath) //dirPath = le chemin du répertoire à supprimer, ex : "/home/user/monRepertoire")
{
    QDir folder(dirPath);
    //On va lister dans ce répertoire tous les éléments différents de "." et ".."
    //(désignant respectivement le répertoire en cours et le répertoire parent)
    folder.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    foreach(QFileInfo fileInfo, folder.entryInfoList())
    {
        //Si l'élément est un répertoire, on applique la méthode courante à ce répertoire, c'est un appel récursif
        if(fileInfo.isDir())
        {
            if(!removeDir(fileInfo.filePath())) //Si une erreur survient, on retourne false
                qDebug()<<"erreur dans remove du dir "+dirPath;
                return false;
        }
        //Si l'élément est un fichier, on le supprime
        else if(fileInfo.isFile())
        {
            if(!QFile::remove(fileInfo.filePath()))
            {
                //Si une erreur survient, on retourne false
                qDebug()<<"erreur dans remove du file"+dirPath;
                return false;
            }
        }
    }

    //Le dossier est maintenant vide, on le supprime
    if(!folder.rmdir(dirPath))
    {
        //Si une erreur survient, on retourne false
        qDebug()<<"erreur dans remove du dir"+dirPath;
        return false;
    }

    //Sinon, on retourne true
    return true;
}

/*void MainWindow::controlRename(){
    QModelIndex index=ui->treeView->currentIndex();
     if (model->fileInfo(index).isFile()){
            QString path = model->filePath(index);
            QString name = model->fileName(index);
            QString dir = path;
            dir.remove(dir.size() - name.size(), name.size());
            QFile file(path);
            if(file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                //Interact with the file
                file.close();
                if(file.rename(QString("%1read %2").arg(dir, name)))
                        qDebug() << "Renamed";
            }
     }
}*/
void MainWindow::reordonneGroup(){
      QModelIndex index=model->index(namedir);
     if (model->fileInfo(index).isDir()){
         QString path = model->filePath(index);
         QString name = model->fileName(index);
         qDebug() << "le path du dossier est "+ path;
         qDebug() << "le nom du dossier est "+ name;

     }



}

extern "C" int* parser_file(const char* name);

// supprimer  le plan 2D Lors d'un double clic sur un nouveau motif
void MainWindow::doubleClick(){
     QModelIndex index=ui->treeView->currentIndex();
     if (model->fileInfo(index).isFile()) {
        dirOrFile=false;

        //réactiver la sélection des plans
        ui->plane1->setDisabled(false);
        ui->plane2->setDisabled(false);
        ui->plane3->setDisabled(false);
        ui->plane4->setDisabled(false);
        ui->plane5->setDisabled(false);
        ui->plane6->setDisabled(false);
        ui->plane7->setDisabled(false);
        ui->plane8->setDisabled(false);
        ui->plane9->setDisabled(false);


         QString name=model->fileInfo(index).absoluteFilePath();
         if(name.compare(this->getEmplMotif())!=0){
             this->setEmpMotif(name);
             qDebug ()<< "nouveau motif "+this->getEmplMotif();
             this->c=Cube();
             deletePlanLed(1);
             desactivePlan(1);

             this->liste_vecteur3D.clear();
             this->ui->widget->setListPoints(liste_vecteur3D);
             ui->widget->setListPlan(liste_vecteur3D);
             int* tab;


           //std::string nameStd = name.toStdString();
           //const char* nomFichier= nameStd.c_str();
           //tab=parser_file(nomFichier);

           GestionFichier ges;

           //QList<QVector3D> l=ges.tabToVector3D(tab);


            QList<QVector3D> l=ges.parser(name);
             if(!l.empty()){

                 this->ui->widget->setListPoints(l);

                 for (QVector3D u:l){
                   Led l=this->c.getList1()->value(u.y()).getLed(fabs(8-u.z()),fabs(8-u.x()));
                   l.modifierEtat();
                   Plan p=c.getList1()->value(u.y());
                   p.updatePlan(l,fabs(8-u.z()),fabs(8-u.x()),u.y());
                   this->c.updateCube(p,u.y());
                   liste_vecteur3D.append(u);
                   this->ui->widget->setListPoints(liste_vecteur3D);
                 }
             }


         }
    else {
        dirOrFile=true;
    }
}
}




void MainWindow::desactivePlan(int niemefois){


            //déselectionner les plans
            ui->plane1->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
             ui->plane2->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
              ui->plane3->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
               ui->plane4->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
                 ui->plane5->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
                  ui->plane6->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
                   ui->plane7->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
                    ui->plane8->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
                    ui->plane9->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");


}


void MainWindow::affichePlanLed(const QString & valeur){

    ui->plane1->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
     ui->plane2->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
      ui->plane3->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
       ui->plane4->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
         ui->plane5->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
          ui->plane6->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
           ui->plane7->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
            ui->plane8->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");
            ui->plane9->setStyleSheet("QPushButton { background-color: rgba(240,240,240,255); }");

    QString stnplan=valeur[1];

    int nplan=stnplan.toInt(0,10);

    this->setNumeroPlan(nplan);

    switch(nplan){
    case 0:ui->plane1->setStyleSheet("QPushButton { background-color: red; }"); break;
    case 1:ui->plane2->setStyleSheet("QPushButton { background-color: red; }"); break;
    case 2:ui->plane3->setStyleSheet("QPushButton { background-color: red; }"); break;
    case 3:ui->plane4->setStyleSheet("QPushButton { background-color: red; }"); break;
    case 4:ui->plane5->setStyleSheet("QPushButton { background-color: red; }"); break;
    case 5:ui->plane6->setStyleSheet("QPushButton { background-color: red; }"); break;
    case 6:ui->plane7->setStyleSheet("QPushButton { background-color: red; }"); break;
    case 7:ui->plane8->setStyleSheet("QPushButton { background-color: red; }"); break;
    case 8:ui->plane9->setStyleSheet("QPushButton { background-color: red; }"); break;
    }

    QList<QVector3D> ll;
    for(int i=0;i<9;i++)
        for(int k=0;k<9;k++)
        {QVector3D v;
            v= QVector3D(i,nplan,k);
            ll.append(v);
            this->ui->widget->setListPlan(ll);
        }

   for (int i = 0; i < 9; i++) {
        for (int j=0;j<9; j++){
           QString col=QString::number(i);
           QString lig=QString::number(j);
           QString text=lig+col;

           int num=text.toInt(0,10);
           buttons[num]->setVisible(true);

           Led l;
           l=c.getList1()->value(this->NumeroPlan).getLed(j,i);

           if(l.getEtat()==0){
               buttons[num]->setIcon(QIcon(":/icone/nvatomeblanc.png"));
           }
           else {
               buttons[num]->setIcon(QIcon(":/icone/atome.gif"));
           }
       }
    }
}

void MainWindow::deletePlanLed(int nfois){


        for (int i = 0; i < 9; i++) {
             for (int j=0;j<9; j++){
                 QString col=QString::number(i);
                 QString lig=QString::number(j);
                 QString text=lig+col;
                 int num=text.toInt(0,10);
                  if(nfois==0){
                         buttons[num]=new QPushButton("",this);
                         buttons[num]->setGeometry(30, 30, 30, 30);
                         buttons[num]->move(30*i+320, 30*j+90);
                  }
                 buttons[num]->hide();
             }
    }
}

void MainWindow::controlLed(const QString & valeur){
   QString strlig=valeur[0];
   QString strcol=valeur[1];
   int lig=strlig.toInt(0,10);
   int col=strcol.toInt(0,10);
  Led l;
  l= this->c.getList1()->value(NumeroPlan).getLed(lig,col);
  l.modifierEtat();

  Plan p1=c.getList1()->value(this->NumeroPlan);
  p1.updatePlan(l,lig,col,NumeroPlan);
  c.updateCube(p1,NumeroPlan);

  QVector3D v;
  v=QVector3D(abs(8-col),NumeroPlan,abs(8-lig));


  liste_vecteur3D.append(v);
  this->ui->widget->setListPoints(liste_vecteur3D);

  if(this->c.getList1()->value(NumeroPlan).getLed(lig,col).getEtat()==1){

        liste_vecteur3D.append(v);
        this->ui->widget->setListPoints(liste_vecteur3D);

  }
  else {
      liste_vecteur3D.removeAll(v);
      this->ui->widget->setListPoints(liste_vecteur3D);
  }




  afficheLed(lig,col,l.getEtat());

}


void MainWindow:: afficheLed(const int i, const int j,const  int etat )
{
       QString lig=QString::number(i);
       QString col=QString::number(j);
       QString text=lig+col;
       int num=text.toInt(0,10);

    if(etat==0){
        this->buttons[num]->setIcon(QIcon(":/icone/nvatomeblanc.png"));
      }
    else {
        this->buttons[num]->setIcon(QIcon(":/icone/atome.gif"));
    }

}

void MainWindow::connexion(){

    QSignalMapper *signalMapper = new QSignalMapper(this);



        connect(ui->plane1, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane1, "00");
        connect(ui->plane2, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane2, "01");
        connect(ui->plane3, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane3, "02");
        connect(ui->plane4, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane4, "03");
        connect(ui->plane5, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane5, "04");
        connect(ui->plane6, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane6, "05");
        connect(ui->plane7, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane7, "06");
        connect(ui->plane8, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane8, "07");
        connect(ui->plane9, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(ui->plane9, "08");

    connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(affichePlanLed(const QString &)));

    QSignalMapper *signalMapper1 = new QSignalMapper(this);
    for (int i = 0; i < 9; i++) {
         for (int j=0;j<9; j++){
            QString col=QString::number(i);
            QString lig=QString::number(j);
            QString text=lig+col;

            int num=text.toInt(0,10);
            connect(buttons[num], SIGNAL(clicked()), signalMapper1, SLOT(map()));
            signalMapper1->setMapping(buttons[num], text);
        }
    }
     connect(signalMapper1, SIGNAL(mapped(const QString &)), this, SLOT(controlLed(const QString &)));


}

int MainWindow::getOrientationPlan()
{
   return OrienPlan;
}

void MainWindow::setOrientationPlan(int i)
{
   OrienPlan=i;
}

int MainWindow::getNumeroPlan()
{
   return NumeroPlan;
}

void MainWindow::setNumeroPlan(int i)
{
   NumeroPlan=i;
}

void MainWindow::setEmpMotif(QString nom){
    this->emplMotif=nom;
}

QString MainWindow::getEmplMotif(){
    return this->emplMotif;
}


MainWindow::~MainWindow()
{
    delete ui;
}




