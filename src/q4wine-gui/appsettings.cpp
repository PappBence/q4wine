/***************************************************************************
 *   Copyright (C) 2008, 2009, 2010 by Malakhov Alexey                           *
 *   brezerk@gmail.com                                                     *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/

#include "appsettings.h"

AppSettings::AppSettings(QWidget * parent, Qt::WFlags f) : QDialog(parent, f)
{
    // Loading libq4wine-core.so
    libq4wine.setFileName("libq4wine-core");

    if (!libq4wine.load()){
        libq4wine.load();
    }

    // Getting corelib calss pointer
    CoreLibClassPointer = (CoreLibPrototype *) libq4wine.resolve("createCoreLib");
    CoreLib.reset((corelib *)CoreLibClassPointer(true));

    setupUi(this);

    splitter.reset(new QSplitter(widgetContent));
    splitter->addWidget(optionsTreeWidget);
    splitter->addWidget(optionsStack);

    QList<int> size;
    size << 150 << 150;

    splitter->setSizes(size);

    std::auto_ptr<QVBoxLayout> vlayout (new QVBoxLayout);
    vlayout->addWidget(splitter.release());
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    widgetContent->setLayout(vlayout.release());

    setWindowTitle(tr("%1 settings").arg(APP_NAME));
    lblCaption->setText(tr("%1 settings").arg(APP_NAME));

    connect(optionsTree, SIGNAL(itemClicked (QTreeWidgetItem *, int)), this, SLOT(optionsTree_itemClicked ( QTreeWidgetItem *, int)));

    connect(cmdCancel, SIGNAL(clicked()), this, SLOT(cmdCancel_Click()));
    connect(cmdOk, SIGNAL(clicked()), this, SLOT(cmdOk_Click()));
    connect(cmdHelp, SIGNAL(clicked()), this, SLOT(cmdHelp_Click()));

    connect(comboProxyType, SIGNAL(currentIndexChanged(QString)), this, SLOT(comboProxyType_indexChanged(QString)));
    connect(radioDefault, SIGNAL(toggled(bool)), this, SLOT(radioDefault_toggled(bool)));
    connect(radioDefaultGui, SIGNAL(toggled(bool)), this, SLOT(radioDefaultGui_toggled(bool)));
    connect(radioFuse, SIGNAL(toggled(bool)), this, SLOT(radioFuse_toggled(bool)));
    connect(radioEmbedded, SIGNAL(toggled(bool)), this, SLOT(radioEmbedded_toggled(bool)));

    //Installing event filters for get buttuns
    cmdGetWineBin->installEventFilter(this);
    cmdGetWineServerBin->installEventFilter(this);
    cmdGetWineLoaderBin->installEventFilter(this);
    cmdGetWineLibs->installEventFilter(this);
    cmdGetTarBin->installEventFilter(this);
    cmdGetMountBin->installEventFilter(this);
    cmdGetUmountBin->installEventFilter(this);
    cmdGetSudoBin->installEventFilter(this);
    cmdGetGuiSudoBin->installEventFilter(this);
    cmdGetNiceBin->installEventFilter(this);
    cmdGetReniceBin->installEventFilter(this);
    cmdGetConsoleBin->installEventFilter(this);
    cmdGetShBin->installEventFilter(this);

    QSettings settings(APP_SHORT_NAME, "default");

    settings.beginGroup("wine");
    txtWineBin->setText(settings.value("WineBin").toString());
    txtWineServerBin->setText(settings.value("ServerBin").toString());
    txtWineLoaderBin->setText(settings.value("LoaderBin").toString());
    txtWineLibs->setText(settings.value("WineLibs").toString());
    settings.endGroup();

    settings.beginGroup("logging");
    if (settings.value("enable", 0).toInt()==1){
        cbEnableLogging->setCheckState(Qt::Checked);
    } else {
        cbEnableLogging->setCheckState(Qt::Unchecked);
    }

    connect (cbEnableLogging, SIGNAL(stateChanged (int)), this, SLOT(cbEnableLogging_stateChanged (int)));
    connect (cbShowTray, SIGNAL(stateChanged (int)), this, SLOT(cbShowTray_stateChanged (int)));

    if (settings.value("autoClear", 1).toInt()==1){
        cbClearLogs->setCheckState(Qt::Checked);
    } else {
        cbClearLogs->setCheckState(Qt::Unchecked);
    }
    settings.endGroup();

    settings.beginGroup("app");
    if (settings.value("showTrareyIcon", 0).toInt()==1){
        cbShowTray->setCheckState(Qt::Checked);
    } else {
        cbShowTray->setCheckState(Qt::Unchecked);
    }

    if (settings.value("showNotifications", 1).toInt()==1){
        cbShowNotifications->setCheckState(Qt::Checked);
    } else {
        cbShowNotifications->setCheckState(Qt::Unchecked);
    }

    if (settings.value("minimizeToTray", 0).toInt()==1){
        cbMinimizeToTray->setCheckState(Qt::Checked);
    } else {
        cbMinimizeToTray->setCheckState(Qt::Unchecked);
    }

    if (settings.value("minimizeToTrayAtAppStart", 0).toInt()==1){
        cbMinimizeToTrayApp->setCheckState(Qt::Checked);
    } else {
        cbMinimizeToTrayApp->setCheckState(Qt::Unchecked);
    }

        listThemesView->clear();

    std::auto_ptr<QListWidgetItem> iconItem (new QListWidgetItem(listThemesView, 0));
    iconItem->setText("Default [Aughtor: Xavier Corredor Llano (xavier.corredor.llano@gmail.com); License: GPL v.2.1]");
    iconItem->setIcon(QIcon(":data/wine.png"));
    iconItem->setToolTip("Default");
    listThemesView->setSelectionMode(QAbstractItemView::SingleSelection);

    QString themeDir="";
    themeDir.append(QDir::homePath());
    themeDir.append("/.config/");
    themeDir.append(APP_SHORT_NAME);
    themeDir.append("/theme/");


    getThemes(settings.value("theme").toString(), themeDir);

    loadThemeIcons();

    if (settings.value("theme").toString()=="Default"){
        listThemesView->setCurrentItem(iconItem.release());
    }

    themeDir.clear();
    themeDir.append(APP_PREF);
    themeDir.append("/share/");
    themeDir.append(APP_SHORT_NAME);
    themeDir.append("/theme");

    getThemes(settings.value("theme").toString(), themeDir);

    getLangs();

    if (settings.value("lang").toString().isEmpty()){
        comboLangs->setCurrentIndex(comboLangs->findText(tr("System Default")));
    } else {
        comboLangs->setCurrentIndex(comboLangs->findText(settings.value("lang").toString()));
    }

    settings.endGroup();

    settings.beginGroup("system");
    txtTarBin->setText(settings.value("tar").toString());
    txtMountBin->setText(settings.value("mount").toString());
    txtUmountBin->setText(settings.value("umount").toString());
    txtSudoBin->setText(settings.value("sudo").toString());
    txtGuiSudoBin->setText(settings.value("gui_sudo").toString());
    txtNiceBin->setText(settings.value("nice").toString());
    txtReniceBin->setText(settings.value("renice").toString());
    txtShBin->setText(settings.value("sh").toString());
    settings.endGroup();

    settings.beginGroup("console");
    txtConsoleBin->setText(settings.value("bin").toString());
    txtConsoleArgs->setText(settings.value("args").toString());
    settings.endGroup();

    settings.beginGroup("quickmount");

    switch (settings.value("type").toInt()){
 case 0:
        //Fix for field update.
        radioDefaultGui->setChecked(true);
        radioDefault->setChecked(true);
        break;
 case 1:
        radioDefaultGui->setChecked(true);
        break;
 case 2:
        radioFuse->setChecked(true);
        break;
 case 3:
        radioEmbedded->setChecked(true);
        break;
    }

    if (!settings.value("mount_drive_string").toString().isEmpty())
        txtMountString->setText(settings.value("mount_drive_string").toString());
    if (!settings.value("mount_image_string").toString().isEmpty())
        txtMountImageString->setText(settings.value("mount_image_string").toString());
    if (!settings.value("umount_string").toString().isEmpty())
        txtUmountString->setText(settings.value("umount_string").toString());

    settings.endGroup();

#ifdef WITH_ICOUTILS
    settings.beginGroup("icotool");
    txtWrestoolBin->setText(settings.value("wrestool").toString());
    txtIcotoolBin->setText(settings.value("icotool").toString());
    settings.endGroup();
    cmdGetWrestoolBin->installEventFilter(this);
    cmdGetIcotoolBin->installEventFilter(this);
#else
    txtWrestoolBin->setEnabled(false);
    txtIcotoolBin->setEnabled(false);
    cmdGetWrestoolBin->setEnabled(false);
    cmdGetIcotoolBin->setEnabled(false);
#endif

    settings.beginGroup("network");
    txtProxyHost->setText(settings.value("host").toString());
    txtProxyPort->setText(settings.value("port").toString());
    txtProxyUser->setText(settings.value("user").toString());
    txtProxyPass->setText(settings.value("pass").toString());

    comboProxyType->setCurrentIndex(settings.value("type").toInt());

    settings.endGroup();

#ifdef _OS_FREEBSD_
    radioFuse->setEnabled(FALSE);
    radioEmbedded->setEnabled(FALSE);
#endif

    settings.beginGroup("advanced");
    if (settings.value("openRunDialog", 0).toInt()==0){
        cbOpenRunDialog->setChecked(false);
    } else {
        cbOpenRunDialog->setChecked(true);
    }

    if (settings.value("useSingleClick", 0).toInt()==0){
        cbUseSingleClick->setChecked(false);
    } else {
        cbUseSingleClick->setChecked(true);
    }

#if QT_VERSION >= 0x040500
    if (settings.value("useNativeFileDialog", 1).toInt()==1){
        cbUseNativeDialog->setChecked(true);
    } else {
        cbUseNativeDialog->setChecked(false);
    }
#else
    cbUseNativeDialog->setEnabled(false);
#endif

    QString res = settings.value("defaultDesktopSize").toString();
    if (res.isEmpty()){
        cboxDesktopSize->setCurrentIndex(0);
    } else {
        cboxDesktopSize->setCurrentIndex(cboxDesktopSize->findText(res));
    }

    settings.endGroup();


    settings.beginGroup("DesktopImport");
    if (settings.value("remove", 0).toInt()==0){
        cbRemoveDesktopFiles->setChecked(false);
    } else {
        cbRemoveDesktopFiles->setChecked(true);
    }

    if (settings.value("importAtStartup", 0).toInt()==0){
        cbImportDesktopFilesAtStartup->setChecked(false);
    } else {
        cbImportDesktopFilesAtStartup->setChecked(true);
    }
    settings.endGroup();

    settings.beginGroup("AppDB");
    if (settings.value("useSystemBrowser", 1).toInt()==1){
        cbUseSystemBrowser->setChecked(true);
    } else {
        cbUseSystemBrowser->setChecked(false);
    }
    settings.endGroup();


    QList<QTreeWidgetItem *> items = optionsTree->findItems (tr("General"), Qt::MatchExactly);
    if (items.count()>0){
        items.at(0)->setExpanded(true);
        optionsTree->setCurrentItem(items.at(0));
        optionsStack->setCurrentIndex(0);
        tabwGeneral->setCurrentIndex(0);
    }

    cmdOk->setFocus(Qt::ActiveWindowFocusReason);
    return;
}

void AppSettings::optionsTree_itemClicked ( QTreeWidgetItem *item, int){
    if (!item)
        return;

    item->setExpanded(true);

    QString itemText = item->text(0);

    if (itemText==tr("General")){
        optionsStack->setCurrentIndex(0);
        tabwGeneral->setCurrentIndex(0);
    } else if (itemText==tr("System")){
        optionsStack->setCurrentIndex(0);
        tabwGeneral->setCurrentIndex(1);
    } else if (itemText==tr("Utils")){
        optionsStack->setCurrentIndex(0);
        tabwGeneral->setCurrentIndex(2);
    } else if (itemText==tr("Network")){
        optionsStack->setCurrentIndex(0);
        tabwGeneral->setCurrentIndex(3);
    } else if (itemText==tr("Quick Mount")){
        optionsStack->setCurrentIndex(0);
        tabwGeneral->setCurrentIndex(4);
    } else if (itemText==tr("Interface")){
        optionsStack->setCurrentIndex(1);
        tabwInterface->setCurrentIndex(0);
    } else if (itemText==tr("Language")){
        optionsStack->setCurrentIndex(1);
        tabwInterface->setCurrentIndex(0);
    } else if (itemText==tr("Themes")){
        optionsStack->setCurrentIndex(1);
        tabwInterface->setCurrentIndex(1);
    } else if (itemText==tr("Subsystems")){
        optionsStack->setCurrentIndex(2);
        tabwInterface->setCurrentIndex(0);
    } else if (itemText==tr("Logging")){
        optionsStack->setCurrentIndex(2);
        tabwSubsystems->setCurrentIndex(0);
    } else if (itemText==tr("AppDb browser")){
        optionsStack->setCurrentIndex(2);
        tabwSubsystems->setCurrentIndex(1);
    } else if (itemText==tr("Plugins")){
        optionsStack->setCurrentIndex(3);
        tabwPlugins->setCurrentIndex(1);
    } else if (itemText==tr("Winetricks")){
        optionsStack->setCurrentIndex(3);
        tabwPlugins->setCurrentIndex(1);
    } else if (itemText==tr("Advanced")){
        optionsStack->setCurrentIndex(4);
        pabwAdvanced->setCurrentIndex(0);
    } else if (itemText==tr("Defaults")){
        optionsStack->setCurrentIndex(4);
        pabwAdvanced->setCurrentIndex(0);
    } else if (itemText==tr("Run dialog")){
        optionsStack->setCurrentIndex(4);
        pabwAdvanced->setCurrentIndex(1);
    } else if (itemText==tr("Wine desktop import")){
        optionsStack->setCurrentIndex(4);
        pabwAdvanced->setCurrentIndex(2);
    }
}

bool AppSettings::eventFilter(QObject *obj, QEvent *event){
    /*
            function for displaying file\dir dialog
      */

    if (event->type() == QEvent::MouseButtonRelease) {

        QString file="";

#if QT_VERSION >= 0x040500
        QFileDialog::Options options;

        if (CoreLib->getSetting("advanced", "useNativeFileDialog", false, 1)==0)
                options = QFileDialog::DontUseNativeDialog | QFileDialog::DontResolveSymlinks;

        if (obj->objectName().right(3)=="Bin"){
            file = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(),   "All files (*)", 0, options);
        } else {
            file = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::homePath(),  options);
        }
#else
        if (obj->objectName().right(3)=="Bin"){
            file = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(),   "All files (*)");
        } else {
            file = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::homePath());
        }
#endif

        if (!file.isEmpty()){
            QString a="";

            a.append("txt");
            a.append(obj->objectName().right(obj->objectName().length()-6));

            std::auto_ptr<QLineEdit> lineEdit (findChild<QLineEdit *>(a));

            if (lineEdit.get()){
                lineEdit->setText(file);
            } else {
                qDebug("Error getting lineEdit object");
            }

            lineEdit.release();
        }
    }

    return FALSE;
}

void AppSettings::comboProxyType_indexChanged(QString text){
    if (text==tr("No Proxy")){
        txtProxyHost->setEnabled(FALSE);
        txtProxyPort->setEnabled(FALSE);
        txtProxyUser->setEnabled(FALSE);
        txtProxyPass->setEnabled(FALSE);
    } else {
        txtProxyHost->setEnabled(TRUE);
        txtProxyPort->setEnabled(TRUE);
        txtProxyUser->setEnabled(TRUE);
        txtProxyPass->setEnabled(TRUE);
    }

    return;
}

void AppSettings::getLangs(){
    QString themeDir="";
#ifdef RELEASE
    themeDir = APP_PREF;
    themeDir.append("/share/");
    themeDir.append(APP_SHORT_NAME);
#else
    themeDir = APP_BUILD;
#endif
    themeDir.append("/i18n");

    QDir tmp(themeDir);
    tmp.setFilter(QDir::Files | QDir::NoSymLinks);

    QFileInfoList list = tmp.entryInfoList();

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.fileName().right(2)=="qm")
            comboLangs->addItem(fileInfo.fileName().split(".").at(0));
    }
    return;
}

void AppSettings::getThemes(QString selTheme, QString themeDir){
    //Getting installed themes

    QString aughtor="", license="";

    QDir tmp(themeDir);
    tmp.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = tmp.entryInfoList();

    // Getting converted icons from temp directory
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        std::auto_ptr<QListWidgetItem> iconItem (new QListWidgetItem(listThemesView, 0));

        QFile file(QString("%1/%2/theme.info").arg(themeDir).arg(fileInfo.fileName()));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            aughtor = file.readLine();
            aughtor.remove ( "\n" );
            license = file.readLine();
        }

        if ((!aughtor.isEmpty()) and (!license.isEmpty())){
            iconItem->setText(QString("%1 [%2; %3]").arg(fileInfo.fileName()).arg(aughtor).arg(license));
        } else {
            iconItem->setText(fileInfo.fileName());
        }
        iconItem->setToolTip(QString("%1/%2").arg(themeDir).arg(fileInfo.fileName()));
        iconItem->setIcon(QIcon(QString("%1/%2/data/wine.png").arg(themeDir).arg(fileInfo.fileName())));

        if (!selTheme.isNull()){
            if (selTheme==fileInfo.filePath()){
                //Setting selection to selected theme
                listThemesView->setCurrentItem(iconItem.release());
                //Loading pixmaps from theme
                //loadThemeIcons(fileInfo.filePath());
            }
        }
    }

    return;
}


void AppSettings::loadThemeIcons(){
    lblLogo->setPixmap(CoreLib->loadPixmap("data/exec.png"));

    cmdGetWineBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetWineServerBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetWineLoaderBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetWineLibs->setIcon(CoreLib->loadIcon("data/folder.png"));

    cmdGetTarBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetMountBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetUmountBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetSudoBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetGuiSudoBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetNiceBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetReniceBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetShBin->setIcon(CoreLib->loadIcon("data/folder.png"));

    cmdGetConsoleBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetWrestoolBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    cmdGetIcotoolBin->setIcon(CoreLib->loadIcon("data/folder.png"));
    return;
}

void AppSettings::cmdCancel_Click(){
    reject();
    return;
}

void AppSettings::cmdOk_Click(){

    if (!checkEntry(txtWineBin->text(), "wine"))
        return;

    if (!checkEntry(txtWineServerBin->text(), "wine server"))
        return;

    if (!checkEntry(txtWineLoaderBin->text(), "wine loader"))
        return;

    if (!checkEntry(txtWineLibs->text(), "wine library", FALSE))
        return;

    if (!checkEntry(txtTarBin->text(), "tar"))
        return;

    if (!checkEntry(txtMountBin->text(), "mount"))
        return;

    if (!checkEntry(txtUmountBin->text(), "umount"))
        return;

    if (!checkEntry(txtSudoBin->text(), "sudo"))
        return;

    if (!checkEntry(txtGuiSudoBin->text(), "gui_sudo"))
        return;

    if (!checkEntry(txtUmountBin->text(), "nice"))
        return;

    if (!checkEntry(txtUmountBin->text(), "renice"))
        return;

    if (!checkEntry(txtUmountBin->text(), "sh"))
        return;

    if (!checkEntry(txtConsoleBin->text(), "console"))
        return;

#ifdef WITH_ICOUTILS
    if (!checkEntry(txtWrestoolBin->text(), "wrestool"))
        return;

    if (!checkEntry(txtIcotoolBin->text(), "icotool"))
        return;
#endif

    if (comboProxyType->currentText()!=tr("No Proxy")){
        if (txtProxyHost->text().isEmpty()){
            QMessageBox::warning(this, tr("Error"), tr("Sorry, specify proxy host."));
            return;
        }
        if (txtProxyPort->text().isEmpty()){
            QMessageBox::warning(this, tr("Error"), tr("Sorry, specify proxy port."));
            return;
        }
    }

    QSettings settings(APP_SHORT_NAME, "default");

    settings.beginGroup("wine");
    settings.setValue("WineBin", txtWineBin->text());
    settings.setValue("ServerBin", txtWineServerBin->text());
    settings.setValue("LoaderBin", txtWineLoaderBin->text());
    settings.setValue("WineLibs", txtWineLibs->text());
    settings.endGroup();

    settings.beginGroup("logging");
    if (cbEnableLogging->checkState()==Qt::Checked) {
        settings.setValue("enable", 1);
    } else {
        settings.setValue("enable", 0);
    }

    if ((cbClearLogs->checkState()==Qt::Checked) and (cbEnableLogging->checkState()==Qt::Checked)) {
        settings.setValue("autoClear", 1);
    } else {
        settings.setValue("autoClear", 0);
    }
    settings.endGroup();

    settings.beginGroup("app");
    if (cbShowTray->checkState()==Qt::Checked) {
        settings.setValue("showTrareyIcon", 1);
    } else {
        settings.setValue("showTrareyIcon", 0);
    }

    if (cbMinimizeToTray->checkState()==Qt::Checked) {
        settings.setValue("minimizeToTray", 1);
    } else {
        settings.setValue("minimizeToTray", 0);
    }

    if (cbMinimizeToTrayApp->checkState()==Qt::Checked) {
        settings.setValue("minimizeToTrayAtAppStart", 1);
    } else {
        settings.setValue("minimizeToTrayAtAppStart", 0);
    }

    if (cbShowNotifications->checkState()==Qt::Checked) {
        settings.setValue("showNotifications", 1);
    } else {
        settings.setValue("showNotifications", 0);
    }

    if (listThemesView->currentItem()){
        settings.setValue("theme", listThemesView->currentItem()->toolTip());
    } else {
        settings.setValue("theme", "Default");
    }


    if (comboLangs->currentText()==tr("System Default")){
        settings.setValue("lang", "");
    } else {
        settings.setValue("lang", comboLangs->currentText());
    }

    settings.endGroup();
    settings.beginGroup("system");
    settings.setValue("tar", txtTarBin->text());
    settings.setValue("mount", txtMountBin->text());
    settings.setValue("umount", txtUmountBin->text());
    settings.setValue("sudo", txtSudoBin->text());
    settings.setValue("gui_sudo", txtGuiSudoBin->text());
    settings.setValue("nice", txtNiceBin->text());
    settings.setValue("renice", txtReniceBin->text());
    settings.setValue("sh", txtShBin->text());
    settings.endGroup();

    settings.beginGroup("console");
    settings.setValue("bin", txtConsoleBin->text());
    settings.setValue("args", txtConsoleArgs->text());
    settings.endGroup();
#ifdef WITH_ICOUTILS
    settings.beginGroup("icotool");
    settings.setValue("wrestool", txtWrestoolBin->text());
    settings.setValue("icotool", txtIcotoolBin->text());
    settings.endGroup();
#endif

    settings.beginGroup("quickmount");
    if (radioDefault->isChecked()){
        settings.setValue("type", 0);
        if (txtMountString->text().isEmpty()){
            txtMountString->setText(CoreLib->getMountString(0));
        }

        if (txtMountImageString->text().isEmpty()){
            txtMountImageString->setText(CoreLib->getMountImageString(0));
        }

        if (txtUmountString->text().isEmpty()){
            txtUmountString->setText(CoreLib->getUmountString(0));
        }
    }

    if (radioDefaultGui->isChecked()){
        settings.setValue("type", 1);
        if (txtMountString->text().isEmpty()){
            txtMountString->setText(CoreLib->getMountString(1));
        }

        if (txtMountImageString->text().isEmpty()){
            txtMountImageString->setText(CoreLib->getMountImageString(1));
        }

        if (txtUmountString->text().isEmpty()){
            txtUmountString->setText(CoreLib->getUmountString(1));
        }
    }

    if (radioFuse->isChecked()){
        settings.setValue("type", 2);
        if (txtMountString->text().isEmpty()){
            txtMountString->setText(CoreLib->getMountString(2));
        }

        if (txtMountImageString->text().isEmpty()){
            txtMountImageString->setText(CoreLib->getMountImageString(2));
        }

        if (txtUmountString->text().isEmpty()){
            txtUmountString->setText(CoreLib->getUmountString(2));
        }
    }

    if (radioEmbedded->isChecked()){
        settings.setValue("type", 3);
        if (txtMountString->text().isEmpty()){
            txtMountString->setText(CoreLib->getMountString(3));
        }

        if (txtMountImageString->text().isEmpty()){
            txtMountImageString->setText(CoreLib->getMountImageString(3));
        }

        if (txtUmountString->text().isEmpty()){
            txtUmountString->setText(CoreLib->getUmountString(3));
        }
    }

    settings.setValue("mount_drive_string", txtMountString->text());
    settings.setValue("mount_image_string", txtMountImageString->text());
    settings.setValue("umount_string", txtUmountString->text());

    settings.endGroup();

    settings.beginGroup("network");
    settings.setValue("host", txtProxyHost->text());
    settings.setValue("port", txtProxyPort->text());
    settings.setValue("user", txtProxyUser->text());
    settings.setValue("pass", txtProxyPass->text());
    if (comboProxyType->currentText()==tr("No Proxy")){
        settings.setValue("type", 0);
    } else {
        if (comboProxyType->currentText()=="HTTP"){
            settings.setValue("type", 1);
        } else {
            settings.setValue("type", 2);
        }
    }

    settings.endGroup();

    settings.beginGroup("advanced");
    if (cbOpenRunDialog->isChecked()){
        settings.setValue("openRunDialog", 1);
    } else {
        settings.setValue("openRunDialog", 0);
    }

    if (cbUseSingleClick->isChecked()){
        settings.setValue("useSingleClick", 1);
    } else {
        settings.setValue("useSingleClick", 0);
    }

#if QT_VERSION >= 0x040500
    if (cbUseNativeDialog->isChecked()){
        settings.setValue("useNativeFileDialog", 1);
    } else {
        settings.setValue("useNativeFileDialog", 0);
    }
#endif

    QString desktopSize=cboxDesktopSize->currentText();
    if (desktopSize==tr("No virtual desktop"))
        desktopSize="";

    settings.setValue("defaultDesktopSize", desktopSize);

    settings.endGroup();

    settings.beginGroup("DesktopImport");
    if (cbRemoveDesktopFiles->isChecked()){
        settings.setValue("remove", 1);
    } else {
        settings.setValue("remove", 0);
    }

    if (cbImportDesktopFilesAtStartup->isChecked()){
        settings.setValue("importAtStartup", 1);
    } else {
        settings.setValue("importAtStartup", 0);
    }
    settings.endGroup();

    settings.beginGroup("AppDB");
    if (cbUseSystemBrowser->isChecked()){
        settings.setValue("useSystemBrowser", 1);
    } else {
        settings.setValue("useSystemBrowser", 0);
    }
    accept();
    return;
}

bool AppSettings::checkEntry(QString fileName, QString info, bool isFile){
    /*
      *	This function check user entry
      */

    if (fileName.isEmpty()){
        switch (isFile){
  case FALSE:
            QMessageBox::warning(this, tr("Error"), tr("Sorry, specify %1 directory.").arg(info));
            break;
  case TRUE:
            QMessageBox::warning(this, tr("Error"), tr("Sorry, specify %1 binary.").arg(info));
            break;
        }
        return FALSE;
    } else {
        if (!QFile::exists(fileName)){
            switch (isFile){
   case FALSE:
                QMessageBox::warning(this, tr("Error"), tr("Sorry, specified %1 directory not exists.").arg(info));
                break;
   case TRUE:
                QMessageBox::warning(this, tr("Error"), tr("Sorry, specified %1 binary not exists.").arg(info));
                break;
            }
            return FALSE;
        }
    }

    return TRUE;
}

void AppSettings::cmdHelp_Click(){
    QString rawurl;

    std::auto_ptr<QTreeWidgetItem> item (optionsTree->currentItem());
    if (!item.get()){
         item.release();
         return;
    }

    QString itemText = item->text(0);

    if (itemText==tr("General")){
        rawurl = "11-settings.html#general";
    } else if (itemText==tr("System")){
        rawurl = "11-settings.html#sysutils";
    } else if (itemText==tr("Utils")){
        rawurl = "11-settings.html#userutils";
    } else if (itemText==tr("Network")){
        rawurl = "11-settings.html#network";
    } else if (itemText==tr("Quick Mount")){
        rawurl = "11-settings.html#qmount";
    } else if (itemText==tr("Interface")){
        rawurl = "11-settings.html#interface";
    } else if (itemText==tr("Language")){
        rawurl = "11-settings.html#interface";
    } else if (itemText==tr("Themes")){
        rawurl = "11-settings.html#interface";
    } else if (itemText==tr("Subsystems")){
        rawurl = "11-settings.html#logging";
    } else if (itemText==tr("Logging")){
        rawurl = "11-settings.html#logging";
    } else if (itemText==tr("AppDb browser")){
        rawurl = "11-settings.html#appdb";
    } else if (itemText==tr("Plugins")){
        rawurl = "11-settings.html";
    } else if (itemText==tr("Winetricks")){
        rawurl = "11-settings.html";
    } else if (itemText==tr("Advanced")){
        rawurl = "11-settings.html#defaults";
    } else if (itemText==tr("Defaults")){
        rawurl = "11-settings.html#defaults";
    } else if (itemText==tr("Run dialog")){
        rawurl = "11-settings.html#rundialog";
    } else if (itemText==tr("Wine desktop import")){
        rawurl = "11-settings.html#wineimport";
    }

    item.release();

    CoreLib->openHelpUrl(rawurl);
    return;
}

void AppSettings::radioDefault_toggled(bool state){
    if (!state)
        return;

    txtMountString->setText(CoreLib->getMountString(0));
    txtMountImageString->setText(CoreLib->getMountImageString(0));
    txtUmountString->setText(CoreLib->getUmountString(0));
    return;
}

void AppSettings::radioDefaultGui_toggled(bool state){
    if (!state)
        return;

    txtMountString->setText(CoreLib->getMountString(1));
    txtMountImageString->setText(CoreLib->getMountImageString(1));
    txtUmountString->setText(CoreLib->getUmountString(1));
    return;
}

void AppSettings::radioFuse_toggled(bool state){
    if (!state)
        return;

    if (CoreLib->getWhichOut("fusermount").isEmpty()){
        radioDefault->setChecked(true);
        return;
    }
    if (CoreLib->getWhichOut("fuseiso").isEmpty()){
        radioDefault->setChecked(true);
        return;
    }

    txtMountString->setText(CoreLib->getMountString(2));
    txtMountImageString->setText(CoreLib->getMountImageString(2));
    txtUmountString->setText(CoreLib->getUmountString(2));
    return;
}

void AppSettings::radioEmbedded_toggled(bool state){
    if (!state)
        return;

#ifdef WITH_EMBEDDED_FUSEISO
    if (CoreLib->getWhichOut("fusermount").isEmpty()){
        radioDefault->setChecked(true);
        return;
    }

    txtMountString->setText(CoreLib->getMountString(3));
    txtMountImageString->setText(CoreLib->getMountImageString(3));
    txtUmountString->setText(CoreLib->getUmountString(3));
#else
    QMessageBox::warning(this, tr("Warning"), tr("<p>q4wine was compiled without embedded FuseIso.</p><p>If you wish to compile q4wine with embedded FuseIso add:</p><p> \"-WITH_EMBEDDED_FUSEISO=ON\" to cmake arguments.</p>"));
    radioDefault->setChecked(true);
#endif
    return;
}

void AppSettings::cbEnableLogging_stateChanged ( int state ){
    if (state==0){
        cbClearLogs->setEnabled(false);
    } else {
        cbClearLogs->setEnabled(true);
    }
}

void AppSettings::cbShowTray_stateChanged ( int state ){
    if (state==0){
        cbShowNotifications->setEnabled(false);
    } else {
        cbShowNotifications->setEnabled(true);
    }
}

