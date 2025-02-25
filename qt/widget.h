#ifndef WIDGET_H
#define WIDGET_H


#include "tfile.h"

#include <QWidget>
#include <QModelIndex>
#include <QThread>
#include <QMutex>
#include <QDebug>
#include "historyNavigator.h"


class QLabel ;
class QPushButton ;
class QCheckBox ;
class QVBoxLayout ;
class QLineEdit ;
class QHBoxLayout ;
class QStringListModel ;
class QListView ;
class QVBoxLayout ;
class QAction ;
class QStatusBar ;
class QCompleter ;
class QSystemTrayIcon ;
class QMenu ;

/*class historySaver : public historyNavigator< QString >
{
    Widget * widg_ ;
public:
    historySaver( Widget * widg ) : widg_( widg ) {}

    QString get_back() const {
        widg_->needToSaveSettings = true ;
        return historyNavigator<QString>::get_back() ;
        //emit lineedit_query->textEdited( lineedit_query->text() ) ;
    }
} ;*/


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    static constexpr char qs_ini_filename[] = "./FileSearchSet.ini" ;
    static constexpr qsizetype max_query_compelete_count = 760 ;
    static constexpr size_t max_query_history_count = 380 ;
    static constexpr size_t max_dirs_history_count = 40 ;

private slots:
    void afterWindowShown() ;

private:
    void on_query_edit( const QString & ) ; // lineedit_query->textEdited
    void on_query_returnpressed( ) ; // lineedit_query->returnPressed()
    void on_query_editfinished() ;
    void on_dirsedit_finished() ; // lineedit_dirs ->editingFinished
    void on_hidden_files_checked() ; // chkbx_hiddenFiles ->stateChanged
    void on_history_go_back() ; // button_hist_left ->clicked , action alt-left
    void on_history_go_forward() ; // button_hist_right ->clicked
    void slot_run_action() ; // run_action ->triggered
    void slot_edit_action() ; // edit_action ->triggered
    void slot_openfolder_action() ;
    void item_activated( const QModelIndex & ) ; // listView ->activated
    void item_doubleClicked(const QModelIndex & ) ;
    void add_query_into_complete_list() ;
    void start_build_tree() ;
    void set_history_bttns_visibility() ;

    void on_tree_built() ;
signals:
    void finished() ;
private:
    QLabel * label_query ;
    QLineEdit * lineedit_query ;
    QLabel * label_dirs ;
    QLineEdit * lineedit_dirs ;
    QLabel * label_hiddenFiles ;
    QCheckBox * chkbx_hiddenFiles ;
    QPushButton * button_stop ;
    QPushButton * button_hist_left ;
    QPushButton * button_hist_right ;

    QHBoxLayout * hbox ;
    QVBoxLayout * vbox ;

    QStatusBar * statusBar ;
    QStringListModel * model ;
    QListView * listView ;
    QAction * run_action ;
    QAction * go_back_action ;
    QAction * go_forward_action ;
    QAction * edit_action ;
    QAction * openfolder_action ;
    QCompleter * query_completer = nullptr ;
    QStringList * query_stringlist ;
    QSystemTrayIcon * trayIcon;
    QMenu * trayIconMenu;

    QMutex * pmutex ;

    bool just_built_tree = false ; // for statusBar adding message (do not clear current message)
    bool needToSaveSettings = false ;
    void writeSettings() ;
    void readSettings() ;

    ttrees * trees_ ;

    int timer_id ;

    std::vector< const tfile * > res_vec ;


    historyNavigator< QString > query_history ;
    historyNavigator< QString > dirs_history ;

protected:
    void showEvent(QShowEvent *ev) override
    {
        QWidget::showEvent(ev);
        // Call slot via queued connection so it's called from the UI thread after this method has returned and the window has been shown
        QMetaObject::invokeMethod(this, "afterWindowShown", Qt::ConnectionType::QueuedConnection);
    }
    void resizeEvent( QResizeEvent * ) override ;
    void moveEvent( QMoveEvent * ) override ;
    void timerEvent(QTimerEvent *) override ;
    bool eventFilter(QObject *target, QEvent *event) override ;

public slots:
    void stop_tree_building() { trees_->filestree_build_running_ = 0 ; }
};



class Worker : public QObject {
    Q_OBJECT
    ttrees * wtrees ;
    QMutex * pmutex ;
public:
    Worker( ttrees * trees , QMutex * m ) : QObject() , wtrees( trees ) , pmutex( m )  { }
signals:
    void finished();
public slots:
    void process() ;
} ;

#endif // WIDGET_H
