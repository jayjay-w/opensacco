#include "reportdesignermainwindow.h"
#include "ui_reportdesignermainwindow.h"
#include <QTextEdit>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>

#include <QtGui>
ReportDesignerMainWindow::ReportDesignerMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ReportDesignerMainWindow)
{
	ui->setupUi(this);

	setUnifiedTitleAndToolBarOnMac(true);

	connect (ui->menuWindow, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

	connect (ui->action_New, SIGNAL(triggered()), SLOT(newFile()));
	connect (ui->action_Save, SIGNAL(triggered()), SLOT(save()));
	connect (ui->action_Save_As, SIGNAL(triggered()), SLOT(saveAs()));
	connect (ui->action_Open, SIGNAL(triggered()), SLOT(open()));

	connect (ui->actionAbout, SIGNAL(triggered()), SLOT(about()));

	connect (ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateMenus()));
	windowMapper = new QSignalMapper(this);
	connect (windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));

	updateMenus();
	updateWindowMenu();
}

ReportDesignerMainWindow::~ReportDesignerMainWindow()
{
	delete ui;
}

void ReportDesignerMainWindow::closeEvent(QCloseEvent *event)
{
	ui->mdiArea->closeAllSubWindows();
	if (ui->mdiArea->currentSubWindow()) {
		event->ignore();
	} else {
		event->accept();
	}
}

void ReportDesignerMainWindow::newFile()
{
	ReportDesignerWindow *child = createMdiChild();
	child->newFile();
	child->show();
}

void ReportDesignerMainWindow::open()
{
	QString fileName = QFileDialog::getOpenFileName(this);
	if (!fileName.isEmpty()) {
		QMdiSubWindow *existing = findMdiChild(fileName);
		if (existing) {
			ui->mdiArea->setActiveSubWindow(existing);
			return;
		}

		ReportDesignerWindow *child = createMdiChild();
		if (child->loadFile(fileName)) {
			statusBar()->showMessage(tr("File loaded"), 2000);
			child->show();
		} else {
			child->close();
		}
	}
}

void ReportDesignerMainWindow::save()
{
	if (activeMdiChild() && activeMdiChild()->save())
		statusBar()->showMessage(tr("File saved"), 2000);
}

void ReportDesignerMainWindow::saveAs()
{
	if (activeMdiChild() && activeMdiChild()->saveAs())
		statusBar()->showMessage(tr("File saved"), 2000);
}

void ReportDesignerMainWindow::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)  {
		ReportDesignerWindow *child = createMdiChild();
		child->loadFile(action->data().toString());
	}
}

#ifndef QT_NO_CLIPBOARD
void ReportDesignerMainWindow::cut()
{
	if (activeMdiChild())
		activeMdiChild()->m_editor->cut();
}

void ReportDesignerMainWindow::copy()
{
	if (activeMdiChild())
		activeMdiChild()->m_editor->copy();
}

void ReportDesignerMainWindow::paste()
{
	if (activeMdiChild())
		activeMdiChild()->m_editor->paste();
}
#endif

void ReportDesignerMainWindow::about()
{
	QMessageBox::about(this, tr("About Report Designer"),
			   tr("The <b>Report Designer</b> application is used to design custom reports"
			      " for the Open Sacco project."
			      ));
}

void ReportDesignerMainWindow::updateMenus()
{
	bool hasMdiChild = (activeMdiChild() != 0);
	ui->action_Save->setEnabled(hasMdiChild);
	ui->action_Save_As->setEnabled(hasMdiChild);
#ifndef QT_NO_CLIPBOARD
	ui->action_Paste->setEnabled(hasMdiChild);
#endif
	ui->actionClose->setEnabled(hasMdiChild);
	//ui->act->setEnabled(hasMdiChild);
	ui->actionTile->setEnabled(hasMdiChild);
	ui->actionCascade->setEnabled(hasMdiChild);
	ui->actionNext->setEnabled(hasMdiChild);
	ui->actionPrevious->setEnabled(hasMdiChild);

#ifndef QT_NO_CLIPBOARD
	bool hasSelection = (activeMdiChild() &&
			     activeMdiChild()->m_editor->textCursor().hasSelection());
	ui->action_Cut->setEnabled(hasSelection);
	ui->action_Copy->setEnabled(hasSelection);
#endif
}

void ReportDesignerMainWindow::updateWindowMenu()
{
	ui->menuWindow->clear();
	ui->menuWindow->addAction(ui->actionClose);
	ui->menuWindow->addSeparator();
	ui->menuWindow->addAction(ui->actionTile);
	ui->menuWindow->addAction(ui->actionCascade);
	ui->menuWindow->addSeparator();
	ui->menuWindow->addAction(ui->actionNext);
	ui->menuWindow->addAction(ui->actionPrevious);
	ui->menuWindow->addSeparator();

	QList<QMdiSubWindow *> windows = ui->mdiArea->subWindowList();
	//separatorAct->setVisible(!windows.isEmpty());

	for (int i = 0; i < windows.size(); ++i) {
		ReportDesignerWindow *child = qobject_cast<ReportDesignerWindow *>(windows.at(i)->widget());

		QString text;
		if (i < 9) {
			text = tr("&%1 %2").arg(i + 1)
					.arg(child->userFriendlyCurrentFile());
		} else {
			text = tr("%1 %2").arg(i + 1)
					.arg(child->userFriendlyCurrentFile());
		}
		QAction *action  = ui->menuWindow->addAction(text);
		action->setCheckable(true);
		action ->setChecked(child == activeMdiChild());
		connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
		windowMapper->setMapping(action, windows.at(i));
	}
}

ReportDesignerWindow *ReportDesignerMainWindow::createMdiChild()
{
	ReportDesignerWindow *child = new ReportDesignerWindow;
	ui->mdiArea->addSubWindow(child);

#ifndef QT_NO_CLIPBOARD
	connect(child, SIGNAL(copyAvailable(bool)),
		ui->action_Cut, SLOT(setEnabled(bool)));
	connect(child, SIGNAL(copyAvailable(bool)),
		ui->action_Copy, SLOT(setEnabled(bool)));
#endif
	return child;
}

void ReportDesignerMainWindow::readSettings()
{
	QSettings settings("freesoftware.co.ke", "Report Designer");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
	move(pos);
	resize(size);
}

void ReportDesignerMainWindow::writeSettings()
{
	QSettings settings("freesoftware.co.ke", "Report Designer");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
}

void ReportDesignerMainWindow::updateRecentFileActions()
{
	/*QSettings settings("freesoftware.co.ke", "Report Designer");
	QStringList files = settings.value("recentFileList").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(files[i]);
		recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		recentFileActs[j]->setVisible(false);*/

	//separatorAct->setVisible(numRecentFiles > 0);
}

ReportDesignerWindow *ReportDesignerMainWindow::activeMdiChild()
{
	if (QMdiSubWindow *activeSubWindow = ui->mdiArea->activeSubWindow())
		return qobject_cast<ReportDesignerWindow *>(activeSubWindow->widget());
	return 0;
}

QMdiSubWindow *ReportDesignerMainWindow::findMdiChild(const QString &fileName)
{
	QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

	foreach (QMdiSubWindow *window, ui->mdiArea->subWindowList()) {
		ReportDesignerWindow *mdiChild = qobject_cast<ReportDesignerWindow *>(window->widget());
		if (mdiChild->currentFile() == canonicalFilePath)
			return window;
	}
	return 0;
}

void ReportDesignerMainWindow::switchLayoutDirection()
{
	if (layoutDirection() == Qt::LeftToRight)
		qApp->setLayoutDirection(Qt::RightToLeft);
	else
		qApp->setLayoutDirection(Qt::LeftToRight);
}

void ReportDesignerMainWindow::setActiveSubWindow(QWidget *window)
{
	if (!window)
		return;
	ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}


/*END OF MAIN WINDOW */


ReportDesignerWindow::ReportDesignerWindow(QWidget *parent)
{
	Q_UNUSED(parent);
	setAttribute(Qt::WA_DeleteOnClose);
	isUntitled = true;
	m_editor = new QTextEdit(this);

	setCentralWidget(m_editor);
}

ReportDesignerWindow::~ReportDesignerWindow()
{

}

void ReportDesignerWindow::newFile()
{
	static int sequenceNumber = 1;

	isUntitled = true;
	curFile = tr("Report %1.xml").arg(sequenceNumber++);
	setWindowTitle(curFile + "[*]");

	connect (m_editor->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
}

bool ReportDesignerWindow::loadFile(const QString &fileName)
{
	QFile file(fileName);

	if (!file.open(QFile::ReadOnly)) {
		QMessageBox::warning(this, tr("Report Designer"),
				     tr("Cannot read file %1:\n%2.")
				     .arg(fileName)
				     .arg(file.errorString()));
		return false;
	}

	QTextStream in(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	m_editor->setPlainText(in.readAll());
	QApplication::restoreOverrideCursor();

	setCurrentFile(fileName);

	connect (m_editor->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));

	return true;
}

bool ReportDesignerWindow::save()
{
	if (isUntitled) {
		return saveAs();
	} else {
		return saveFile(curFile);
	}
}

bool ReportDesignerWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), curFile);

	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

bool ReportDesignerWindow::saveFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Report Designer"),
				     tr("Cannot write file %1:\n%2.")
				     .arg(fileName)
				     .arg(file.errorString()));
		return false;
	}

	QTextStream out(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	out << m_editor->toPlainText();
	QApplication::restoreOverrideCursor();

	setCurrentFile(fileName);
	return true;
}

QString ReportDesignerWindow::userFriendlyCurrentFile()
{
	return strippedName(curFile);
}

void ReportDesignerWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave()) {
		event->accept();
	} else {
		event->ignore();
	}
}

void ReportDesignerWindow::documentWasModified()
{
	setWindowModified(m_editor->document()->isModified());
}

bool ReportDesignerWindow::maybeSave()
{
	if (m_editor->document()->isModified()) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("Report Designer"),
					   tr("'%1' has been modified.\n"
					      "Do you want to save your changes?")
					   .arg(userFriendlyCurrentFile()),
					   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

		if (ret == QMessageBox::Save)
			return save();
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	return true;
}

void ReportDesignerWindow::setCurrentFile(const QString &fileName)
{
	curFile = QFileInfo(fileName).canonicalFilePath();
	isUntitled = false;
	m_editor->document()->setModified(false);
	setWindowModified(false);
	setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString ReportDesignerWindow::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}
