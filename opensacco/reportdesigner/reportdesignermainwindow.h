#ifndef REPORTDESIGNERMAINWINDOW_H
#define REPORTDESIGNERMAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
class QTextEdit;
class MdiChild;
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;

namespace Ui {
class ReportDesignerMainWindow;
}

class ReportDesignerWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit ReportDesignerWindow(QWidget *parent = 0);
	~ReportDesignerWindow();

	void newFile();
	bool loadFile(const QString &fileName);
	bool save();
	bool saveAs();
	bool saveFile(const QString &fileName);
	QString userFriendlyCurrentFile();
	QString currentFile() { return curFile; }
	QTextEdit *m_editor;

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void documentWasModified();

private:
	bool maybeSave();
	void setCurrentFile(const QString &fileName );
	QString strippedName(const QString &fullFileName);

	QString curFile;
	bool isUntitled;
};

class ReportDesignerMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit ReportDesignerMainWindow(QWidget *parent = 0);
	~ReportDesignerMainWindow();

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void newFile();
	void open();
	void save();
	void saveAs();
	void openRecentFile();
#ifndef QT_NO_CLIPBOARD
	void cut();
	void copy();
	void paste();
#endif
	void about();
	void updateWindowMenu();
	void updateMenus();
	ReportDesignerWindow *createMdiChild();
	void switchLayoutDirection();
	void setActiveSubWindow(QWidget *window);

private:
	Ui::ReportDesignerMainWindow *ui;
	void readSettings();
	void writeSettings();

	void updateRecentFileActions();

	ReportDesignerWindow *activeMdiChild();
	QMdiSubWindow *findMdiChild(const QString &fileName);

	QSignalMapper *windowMapper;

	enum { MaxRecentFiles  =5 };
	QAction *recentFileActs[MaxRecentFiles];
};

#endif // REPORTDESIGNERMAINWINDOW_H
