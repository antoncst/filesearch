
//#include <QtGui>
#include "widget.h"
#include "tfile.h"
#include <sstream>
#include <filesystem>


#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QStatusBar>
#include <QBoxLayout>
#include <QStringListModel>
#include <QListView>
#include <QResizeEvent>
#include <QTimerEvent>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QModelIndex>
#include <QAction>
#include <QDesktopServices>
#include <QCompleter>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QMenu>

//#include <QTime>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    trayIcon = new QSystemTrayIcon ;
    trayIcon->setVisible( true ) ;

    label_query = new QLabel( tr("&Запрос:") ) ;
    lineedit_query = new QLineEdit() ;
    label_query->setBuddy( lineedit_query ) ;

    label_dirs = new QLabel( tr("&Папки:") ) ;
    lineedit_dirs = new QLineEdit() ;
    label_dirs->setBuddy( lineedit_dirs ) ;

    label_hiddenFiles = new QLabel( tr("Скрытые") ) ;
    chkbx_hiddenFiles = new QCheckBox() ;
    chkbx_hiddenFiles->setFocusPolicy( Qt::NoFocus ) ;
    label_hiddenFiles->setBuddy( chkbx_hiddenFiles ) ;

    button_stop = new QPushButton( tr( "Стоп" ) ) ;
    button_stop->setFocusPolicy( Qt::NoFocus ) ;
    button_stop->setVisible( false ) ;

    button_hist_left = new QPushButton( "◀" ) ;
    button_hist_left->setToolTip( tr( "Назад (Alt-Left)" ) ) ;
    button_hist_left->setMaximumWidth( 20 ) ;
    button_hist_left->setFocusPolicy( Qt::NoFocus ) ;

    button_hist_right = new QPushButton( "▶" ) ;
    button_hist_right->setToolTip( tr( "Вперед (Alt-Right)" ) ) ;
    button_hist_right->setMaximumWidth( 20 ) ;
    button_hist_right->setFocusPolicy( Qt::NoFocus ) ;

    hbox = new QHBoxLayout() ;
    hbox->addWidget( label_query ) ;
    hbox->addWidget( lineedit_query ) ;
    hbox->addWidget( label_dirs ) ;
    hbox->addWidget( lineedit_dirs ) ;
    hbox->addWidget( label_hiddenFiles ) ;
    hbox->addWidget( chkbx_hiddenFiles ) ;
    hbox->addWidget( button_stop ) ;
    hbox->addWidget( button_hist_left ) ;
    hbox->addWidget( button_hist_right ) ;


    trees_ = new ttrees ;

    statusBar = new QStatusBar() ;
    //model = new QStringListModel( this ) ;
    listView = new QListView() ;

    vbox = new QVBoxLayout ;
    vbox->addLayout( hbox ) ;
    vbox->addWidget( listView ) ;
    vbox->addWidget( statusBar ) ;

    setLayout(vbox);

    lineedit_query->installEventFilter( this ) ;
    connect( lineedit_query  , &QLineEdit::textEdited ,
        this, &Widget::on_query_edit ) ; //( const QString & )
    connect( lineedit_query  , &QLineEdit::returnPressed ,
            this, &Widget::on_query_returnpressed ) ; //( const QString & )
    connect( lineedit_query  , &QLineEdit::editingFinished  ,
            this, &Widget::on_query_editfinished ) ;

    connect( lineedit_dirs , &QLineEdit::editingFinished  ,
             this, &Widget::on_dirsedit_finished ) ;

    connect ( chkbx_hiddenFiles , &QCheckBox::stateChanged ,// (int state)
              this , &Widget::on_hidden_files_checked ) ;
    connect( button_hist_left , &QPushButton::clicked  ,
            this , &Widget::on_history_go_back ) ;
    connect( button_hist_right , &QPushButton::clicked  ,
            this , &Widget::on_history_go_forward ) ;

    run_action = new QAction(tr("Открыть файл (Enter)") , this) ; // Run
    run_action->setShortcut( QKeySequence( Qt::ControlModifier | Qt::Key_R ) );
    run_action->setStatusTip(tr("Запустить") ) ;
    connect( run_action , &QAction::triggered  , this , &Widget::slot_run_action  ) ;

    edit_action = new QAction(tr("Режим редактирования") , this) ;
    edit_action->setShortcut( QKeySequence(  Qt::Key_F2 ) );
    edit_action->setStatusTip(tr("Для копирования текста, не для исправления") ) ;
    connect( edit_action , &QAction::triggered  , this , &Widget::slot_edit_action  ) ;

    openfolder_action = new QAction( tr("Открыть папку"), this ) ;
    connect( openfolder_action , &QAction::triggered  , this , &Widget::slot_openfolder_action  ) ;

    query_stringlist = new QStringList() ;

    listView->addAction( run_action ) ;
    listView->addAction( edit_action ) ;
    listView->addAction(  openfolder_action );

    listView->setContextMenuPolicy( Qt::ActionsContextMenu ) ;

    go_back_action = new QAction(tr("Back") , this) ;
    go_back_action->setShortcut( QKeySequence( Qt::AltModifier | Qt::Key_Left ) );
    //go_back_action->setStatusTip(tr("") ) ;
    connect( go_back_action , &QAction::triggered  , this , &Widget::on_history_go_back  ) ;

    go_forward_action = new QAction(tr("Forward") , this) ;
    go_forward_action ->setShortcut( QKeySequence( Qt::AltModifier | Qt::Key_Right ) );
    //go_back_action->setStatusTip(tr("") ) ;
    connect( go_forward_action  , &QAction::triggered  , this , &Widget::on_history_go_forward  ) ;

    addAction( go_back_action ) ;
    addAction( go_forward_action ) ;

    connect( listView , &QListView::activated, this, &Widget::item_activated ) ;
    connect( listView , &QListView::doubleClicked , this, &Widget::item_doubleClicked ) ;

    model = new QStringListModel(this);

    timer_id = startTimer( 1500 ) ; // for writeSettings

    query_history.set_max_size( max_query_history_count ) ;
    dirs_history.set_max_size( max_dirs_history_count );
    readSettings() ;

    set_history_bttns_visibility();
}

void Widget::afterWindowShown()
{
    qDebug() << "afterWindowShown()" ;
    trayIcon->show();
    if ( lineedit_dirs->text() != "" )
        on_dirsedit_finished() ;
        //emit lineedit_dirs->editingFinished() ; //on_dirsedit_finished()

}
void Widget::resizeEvent(QResizeEvent * event)
{
    qDebug() << "resizeEvent" << event->size().width() << event->size().height();
    needToSaveSettings = true ;
    QWidget::resizeEvent(event);
}

void Widget::moveEvent( QMoveEvent * event )
{
    needToSaveSettings = true ;
    QWidget::moveEvent(event);
}


void Widget::timerEvent( QTimerEvent *  )
{
    if ( needToSaveSettings ) writeSettings() ;
    if ( trees_->filestree_build_running_ )
    {
        statusBar->setPalette( QGuiApplication::palette() );
        statusBar->showMessage( tr("Построение дерева файлов: ") + QString::number( trees_->get_count_files() ) + tr("...") ) ;
    }
}

bool Widget::eventFilter(QObject *target, QEvent *event)
{
    if ( target == lineedit_query )
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent * keyEvent = static_cast< QKeyEvent * >( event) ;
            if ( keyEvent->key() == Qt::Key_Down )
            {
                listView->setFocus() ;
                return true;
            }
        }
    }
    return QWidget::eventFilter( target, event ) ;
}

void Widget::writeSettings()
{

    qDebug() << "Saving settings" ;
    QSettings settings( qs_ini_filename , QSettings::IniFormat ) ;
    settings.setValue( "geometry" , geometry() ) ;
    settings.setValue( "rootDirs" , lineedit_dirs->text() ) ;
    settings.setValue( "hiddenFiles" , chkbx_hiddenFiles->checkState() ) ;
    if ( query_stringlist->size() > 0 )
        settings.setValue( "query_stringlist" , * query_stringlist ) ;

    std::vector< QString > history_vector = query_history.get_content() ;
    if ( history_vector.size() > 0 ) {
        //int diff = history_vector.size() - max_query_history_count ;
        //if ( diff < 0 ) diff = 0 ;
        QStringList history_list( history_vector.begin() /* + diff */, history_vector.end() ) ;
        settings.setValue( "history_list" , history_list ) ;

        int cur_item = query_history.get_cur_item_index() ;
        //cur_item -= diff ;
        //if (cur_item < 1 ) cur_item = 1 ;
        settings.setValue( "history_cur_item" , cur_item ) ;
    }

    std::vector< QString > dirs_history_vector = dirs_history.get_content() ;
    if ( dirs_history_vector.size() > 0 ) {
        //int diff = dirs_history_vector.size() - max_dirs_history_count ;
        //if ( diff < 0 ) diff = 0 ;
        QStringList history_list( dirs_history_vector.begin() /* + diff */ , dirs_history_vector.end() ) ;
        settings.setValue( "dirs_history_list" , history_list ) ;
        //do not care about cur_item
    }
    needToSaveSettings = false ;
}

void Widget::readSettings()
{
    qDebug() << "readSettings()" ;
    QRect rect_widget ;
    QSettings settings( qs_ini_filename ,  QSettings::IniFormat);
    if ( settings.contains( "geometry" ) )
    {
        rect_widget = settings.value("geometry").toRect();
        //костыль местоположения для windows
        #ifdef Q_OS_WIN
            int n_amendment = 30 ;
            rect_widget.setTop( rect_widget.top() - n_amendment );
            rect_widget.setHeight(rect_widget.height() - n_amendment ) ;
        #endif
    } else
    {
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect rect_screen = screen->availableGeometry() ;
        rect_widget.setTop( rect_screen.height() / 10 ) ;
        rect_widget.setLeft( rect_screen.right() / 5 ); ;
        rect_widget.setWidth( rect_screen.width() / 2 ) ;
        rect_widget.setHeight( rect_screen.height() / 5 * 4 );
    }
    move(rect_widget.topLeft());
    resize(rect_widget.size());

    if ( settings.contains( "rootDirs" ) )
    {
        lineedit_dirs->setText( settings.value( "rootDirs" ).toString() ) ;
    }
    if ( settings.contains( "hiddenFiles" ) )
    {
        chkbx_hiddenFiles->setCheckState( Qt::CheckState(settings.value("hiddenFiles").toUInt() ) ) ;

    }
    trees_->show_hidden( chkbx_hiddenFiles->checkState() ) ;
#ifdef Q_OS_WIN
    if ( lineedit_dirs->text() == "" )
        lineedit_dirs->setText( "c:\\Users ; "
                               "!:\\Windows ; !:\\Program Files ; !\\AppData\\"
                               "!:\\ProgramData ; !:\\Recovery ; "
                               "" );
#endif

    lineedit_dirs->setCursorPosition( 0 ) ;

    if ( settings.contains( "query_stringlist" ) )
    {
        * query_stringlist = settings.value( "query_stringlist" ).toStringList() ;
        query_completer = new QCompleter( * query_stringlist , this) ;
        query_completer->setCaseSensitivity( Qt::CaseInsensitive ) ;
        query_completer->setCompletionMode( QCompleter::InlineCompletion ) ;
        lineedit_query->setCompleter( query_completer ) ;
        if ( query_stringlist->size() > 0 )
            lineedit_query->setText( query_stringlist->at( 0 ) ) ;
    }

    int cur_item = 1 ;
    if ( settings.contains( "history_cur_item" ) )
        cur_item = settings.value( "history_cur_item" ).toInt() ;

    if ( settings.contains( "history_list" ) )
    {
        QStringList content = settings.value( "history_list" ).toStringList() ;
        if ( content.size() > 0 ) {
            query_history.set_content( std::vector( content.begin() , content.end() ) ) ;
            query_history.set_cur_item( cur_item ) ;
            lineedit_query->setText( query_history.get_cur_item_value() ) ;
            emit lineedit_query->textEdited( lineedit_query->text() ) ;
        }
    }
    if ( settings.contains( "dirs_history_list" ) )
    {
        QStringList content = settings.value( "dirs_history_list" ).toStringList() ;
        if ( content.size() > 0 ) {
            dirs_history.set_content( std::vector( content.begin() , content.end() )) ;
            dirs_history.set_cur_item( content.size() ) ;
        }
    }
}

void Widget::add_query_into_complete_list()
{
    query_stringlist->removeAll( lineedit_query->text() ) ;

    query_stringlist->push_front( lineedit_query->text() ) ;
    while ( query_stringlist->count() > max_query_compelete_count )
        query_stringlist->removeLast() ;


    if ( query_completer )
    {
        delete query_completer ;
    }
    query_completer = new QCompleter( * query_stringlist , this ) ;
    query_completer->setCaseSensitivity( Qt::CaseInsensitive ) ;
    query_completer->setCompletionMode( QCompleter::InlineCompletion ) ;
    lineedit_query->setCompleter( query_completer ) ;
    writeSettings() ;
}

void Widget::item_activated( const QModelIndex & index )
{
    QString str_file = model->data( index ).toString() ;

    QDesktopServices::openUrl( QUrl::fromLocalFile( str_file ) );

    qDebug() << str_file ;
    add_query_into_complete_list() ;
}

void Widget::item_doubleClicked( const QModelIndex & index )
{
    item_activated( index ) ;
}


void Widget::slot_openfolder_action()
{

    QString str_file = model->data( listView->currentIndex() ).toString() ;

    //QDesktopServices::openUrl( QUrl::fromLocalFile( str_file.left( str_file.lastIndexOf( QDir::separator() ) ) ) ) ;
#ifdef Q_OS_WIN
    QString str_folder = str_file.left( str_file.lastIndexOf( QDir::separator() ) ) ;
    QProcess::execute( QString("explorer") , QStringList( str_folder ) ) ;
#elif defined( Q_OS_LINUX )
    QDesktopServices::openUrl( QUrl( "file://"+str_file.left( str_file.lastIndexOf( QDir::separator() ) ) , QUrl::TolerantMode ) ) ;
#else
    QDesktopServices::openUrl( QUrl( "file://"+str_file.left( str_file.lastIndexOf( QDir::separator() ) ) , QUrl::TolerantMode ) ) ;
#endif
    add_query_into_complete_list() ;

}

void Widget::slot_run_action()
{
    item_activated( listView->currentIndex() ) ;
}

void Widget::slot_edit_action()
{
    listView->edit( listView->currentIndex() ) ;
    add_query_into_complete_list() ;
}

//todo on exit add to history the query

void Widget::on_tree_built()
{

    pmutex->lock() ;

    qDebug() << "Found " << trees_->get_count_files() << " files" ;

    statusBar->setPalette( QGuiApplication::palette() ) ;
    QString msg = tr("Найдено файлов и папок: ") + QString::number( trees_->get_count_files() ) ;
    QString msg2 = "" ;
    if ( trees_->undiscovered_skip_dirs_.size() > 0 ) {
        msg2 = tr( ". Необработанные папки: " ) ;
        bool bfirst = true ;
        for ( auto & item : trees_->undiscovered_skip_dirs_ ) {
            if ( !bfirst )
                msg2 += ", " ;
            bfirst = false ;
            msg2 += item ;
        }
    }
    statusBar->showMessage( msg + msg2 ) ;

    pmutex->unlock() ;
    delete pmutex ;

    trees_->filestree_build_running_ = 0 ;
    trees_->app_starting = false ;
    just_built_tree = true ;

    button_stop->setVisible( false ) ;

    lineedit_query->setEnabled( true ) ;
    lineedit_dirs->setEnabled( true ) ;
    lineedit_query->setFocus() ;
    emit lineedit_query->textEdited( lineedit_query->text() ); //on_query_edit

}

void Worker::process()
{
    pmutex->lock() ;
    wtrees->make_tree() ;
    pmutex->unlock() ;

    emit finished() ;
}

void Widget::on_hidden_files_checked()
{
    qDebug() << "event on_hidden_files_checked" ;
    writeSettings() ;
    trees_->show_hidden( chkbx_hiddenFiles->checkState() ) ;
    if ( ! trees_->app_starting )
        if ( lineedit_dirs->text() != "" )
        {
            on_dirsedit_finished() ;
            start_build_tree() ;
        }
    //trees_->set_dirs( lineedit_dirs->text() , true ) ;
}

void Widget::start_build_tree()
{
    if ( trees_->filestree_build_running_ ) {
        qDebug() << " building tree already running " ;
        return ;
    }

    if ( ! trees_->dirs_set_ ) {
        qDebug() << "Error! dirs are not set" ;
        return ;
    }

    trees_->filestree_build_running_ = 1 ;

    qDebug() << "start_build_tree" ;

    statusBar->setPalette( QGuiApplication::palette() );
    statusBar->showMessage( tr("Построение дерева файлов... ") ) ; //+ QString::number( trees_->get_count_files() ) ) ;
    lineedit_query->setEnabled( false ) ;
    lineedit_dirs->setEnabled( false ) ;

    QThread *thread = new QThread;
    pmutex = new QMutex ;
    Worker *w = new Worker( trees_ , pmutex ) ;
    w->moveToThread(thread);
    connect(thread, &QThread::started, w, &Worker::process);
    connect(w, &Worker::finished, this, &Widget::on_tree_built ) ;
    connect(w, &Worker::finished, this, &Widget::finished);
    connect(w, &Worker::finished, thread, &QThread::quit);
    connect(w, &Worker::finished, w, &Worker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater) ;
    connect( button_stop , &QPushButton::clicked  , this , &Widget::stop_tree_building ) ;

    thread->start();

    button_stop->setVisible( true ) ;
}


void Widget::on_dirsedit_finished()
{
    qDebug() << "event on_dirsedit_finished()" ;
    try
    {
        if ( ! trees_->set_dirs( lineedit_dirs->text() ) )
            return ;
    }
    catch ( QString & err_str )
    {
        QPalette error_palette;
        error_palette.setColor(QPalette::WindowText, Qt::red ) ;
        statusBar->setPalette(error_palette);
        statusBar->showMessage( tr("Папка не найдена: ") + err_str ) ;
        return ;
    }
    if ( ! trees_->app_starting )
        dirs_history.add( lineedit_dirs->text() ) ;
    writeSettings() ;
    start_build_tree() ;

}

void Widget::on_query_returnpressed()
{
    //emit lineedit_query->textEdited( lineedit_query->text() ) ;
    // after returnPressed event, editingFinished event appears , so nothing to do
}

void Widget::on_query_editfinished()
{
    emit lineedit_query->textEdited( lineedit_query->text() ) ;
    if ( lineedit_query->text().size() >= 2 )
    {
        query_history.add( lineedit_query->text() ) ;
        set_history_bttns_visibility() ;
        needToSaveSettings = true ;
    }
}

void Widget::on_query_edit( const QString & text )
{
    qDebug() << "on_query_edit( const QString & text )" ;
    if ( text.size() >= 2 )
    {
        listView->reset() ;

        std::vector< const tfile * > res_vec =  trees_->find_str( text ) ;

        statusBar->setPalette( QGuiApplication::palette() );
        QString msg = "" ;
        if ( just_built_tree ) {
            just_built_tree = false ;
            msg = statusBar->currentMessage() + " " ;
        }
        msg += tr("Результатов: ") + QString::number( res_vec.size() ) ;
        statusBar->showMessage( msg ) ;
        qDebug() << "Found " << res_vec.size() << " items" ;

        QStringList res_sl ;
        for ( const tfile * file : res_vec )
            res_sl.append( trees_->get_full_name( file ) ) ;  //todo get_full_name()


        model->setStringList( res_sl ) ;

        listView->setModel(model) ;
    }
}


void Widget::on_history_go_back()
{
    lineedit_query->setText( query_history.get_back() ) ;
    emit lineedit_query->textEdited( lineedit_query->text() ) ;
    needToSaveSettings = true ;
    set_history_bttns_visibility() ;

}

void Widget::on_history_go_forward()
{
    lineedit_query->setText( query_history.get_forward() ) ;
    emit lineedit_query->textEdited( lineedit_query->text() ) ;
    needToSaveSettings = true ;
    set_history_bttns_visibility() ;
}

void Widget::set_history_bttns_visibility()
{
    button_hist_left->setEnabled( query_history.has_back_item() ) ;
    button_hist_right->setEnabled( query_history.has_forward_item() ) ;
}



Widget::~Widget()
{}
