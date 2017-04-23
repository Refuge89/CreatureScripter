#include "templatetables.h"
#include "cache.h"
#include "migration.h"

#include <QVector>
#include <QLineEdit>
#include <QAbstractItemModel>
#include <QSqlRecord>
#include <QSqlField>
#include <QTreeView>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <functional>
#include <QFile>
#include <QFont>
#include <QStack>
#include <utility>
#include <QKeyEvent>

class TemplateTreeItem
{
public:
    enum itemType{ROOT,TABLE,FIELD};
    itemType type;
    QString _table;
    TemplateTreeItem *_parentItem;
    QSqlField _itemData;
    QVariant _editableValue;
    QSqlField _originalData;
    QVector<TemplateTreeItem*> _childItems;

    TemplateTreeItem() : type(ROOT),_parentItem(nullptr){}
    TemplateTreeItem(QString t,TemplateTreeItem* p) : type(TABLE), _table(t),_parentItem(p)
    {
        p->appendChild(this);
    }
    TemplateTreeItem(const QSqlField& f, TemplateTreeItem* p) :
        type(FIELD),
        _parentItem(p),
        _itemData(f),
        _editableValue(f.value()),
        _originalData(f)
    {
        p->appendChild(this);
    }

    ~TemplateTreeItem(){
        qDeleteAll(_childItems);
        _childItems.clear();
    }
    void appendChild(TemplateTreeItem *item)
    {
        _childItems.append(item);
    }
    TemplateTreeItem *child(int row){
        return _childItems.at(row);
    }

    int childCount() const{ return _childItems.count(); }
    int columnCount() const {
        switch(type){
        case ROOT: return 2;
        case TABLE: return 2;
        case FIELD: return 2;
        }
    }
    QVariant data(int column) const{
        switch(type){
        case ROOT: return column ? "Value" : "Field";
        case TABLE: return column ? "" : _table;
        case FIELD: return column ? _editableValue : _itemData.name();
        }
    }
    int row() const {
        if (_parentItem)
            return _parentItem->_childItems.indexOf(const_cast<TemplateTreeItem*>(this));
        return 0;
    }
    TemplateTreeItem *parentItem() {return _parentItem; }
    QVariant SetData(QVariant data){
        Q_ASSERT(type==FIELD);
        if(_editableValue != data){
            QVariant tmp = _editableValue;
            _editableValue.setValue(data);
            return tmp;
        }
        return QVariant(QVariant::Invalid);
    }
    bool Changed(){
        return _originalData.value() != _editableValue;
    }

    bool Match(const QString& match)
    {
        Q_ASSERT(type==FIELD);
        if(match.isEmpty()) return true;
        return _originalData.name().contains(match, Qt::CaseInsensitive)
                || _editableValue.toString().contains(match,Qt::CaseInsensitive);
    }
};

class TemplateTableModel : public QAbstractItemModel
{
private:
    QBrush oddbrush,evenbrush,editedbrush;
    QStack<std::pair<QModelIndex,QVariant>> changeStack;

public:
    TemplateTreeItem* rootItm;
    TemplateTableModel(const QVector<std::pair<QString,QSqlRecord>>& records, QWidget* parent) :
        QAbstractItemModel(parent),
        rootItm(new TemplateTreeItem),
        oddbrush(QBrush(QColor(50,50,50,100))),
        evenbrush(QBrush(QColor(50,50,50,100).darker())),
        editedbrush(QBrush(QColor(200,100,100)))
    {
        for(auto r = records.begin(); r != records.end(); r++){
            TemplateTreeItem* itm = new TemplateTreeItem(r->first, rootItm);
            for(int col = 0; col < r->second.count(); col++){
                new TemplateTreeItem(r->second.field(col), itm);
            }
        }
    }
    ~TemplateTableModel() { rootItm; }

    void PushChange(const QModelIndex &index,const QVariant& v){
        changeStack.push(std::make_pair(index,v));
    }
    QModelIndex PopChange(){
        if(changeStack.isEmpty()) return QModelIndex();
        auto itm = changeStack.pop();
        TemplateTreeItem* tItem = static_cast<TemplateTreeItem*>(itm.first.internalPointer());
        QVariant ret = tItem->SetData(itm.second);
        if(ret.isValid()){
            return itm.first;
        }
        return QModelIndex();
    }

    QModelIndex index(int row, int column, const QModelIndex &parent) const override
    {
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        TemplateTreeItem *parentItem;

        if (!parent.isValid())
            parentItem = rootItm;
        else
            parentItem = static_cast<TemplateTreeItem*>(parent.internalPointer());

        TemplateTreeItem *childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }
    QModelIndex parent(const QModelIndex &index) const
    {
        if (!index.isValid())
            return QModelIndex();

        TemplateTreeItem *childItem = static_cast<TemplateTreeItem*>(index.internalPointer());
        TemplateTreeItem *parentItem = childItem->parentItem();

        if (parentItem == rootItm)
            return QModelIndex();

        return createIndex(parentItem->row(), 0, parentItem);
    }
    int rowCount(const QModelIndex &parent) const
    {
        TemplateTreeItem *parentItem;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid())
            parentItem = rootItm;
        else
            parentItem = static_cast<TemplateTreeItem*>(parent.internalPointer());

        return parentItem->childCount();
    }
    int columnCount(const QModelIndex& parent) const override
    {
        if (parent.isValid())
            return static_cast<TemplateTreeItem*>(parent.internalPointer())->columnCount();
        else
            return rootItm->columnCount();
    }
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid())
            return QVariant();

        TemplateTreeItem *item = static_cast<TemplateTreeItem*>(index.internalPointer());
        /*switch(role)
        {
        case Qt::DisplayRole:
        case Qt::DecorationRole:
            return QVariant();
        case Qt::EditRole:
            break;
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
        // Metadata
        case Qt::FontRole:
        case Qt::TextAlignmentRole:
            return QVariant();
        case Qt::BackgroundColorRole: // Qt::BackgroundRole both 8
            return index.row() % 2 ? oddbrush : evenbrush;
        case Qt::TextColorRole: //Qt::ForegroundRole both 9
        case Qt::CheckStateRole:
        // Accessibility
        case Qt::AccessibleTextRole:
        case Qt::AccessibleDescriptionRole:
        // More general purpose
        case Qt::SizeHintRole:
        case Qt::InitialSortOrderRole:
            return QVariant();
        }
        */

        if(role == Qt::BackgroundColorRole){
            if(item->Changed()){
                return editedbrush;
            }
            return index.row() % 2 ? oddbrush : evenbrush;
        }
        else if (role != Qt::DisplayRole && role != Qt::EditRole){
            return QVariant();
        }
        return item->data(index.column());
    }
    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags f = QAbstractItemModel::flags(index);
        if(static_cast<TemplateTreeItem*>(index.internalPointer())->type == TemplateTreeItem::FIELD && index.column()==1)
            f |= Qt::ItemIsEditable;// | Qt::ItemIsSelectable;

        return f;
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return rootItm->data(section);

        return QVariant();
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (role != Qt::EditRole)
            return false;
        TemplateTreeItem* itm = static_cast<TemplateTreeItem*>(index.internalPointer());
        QVariant oldVal = itm->SetData(value);
        if(oldVal.isValid()){
            PushChange(index, oldVal);
            return true;
        }
        return false;
    }

    Migrations GetMigrations()
    {
        Migrations mig;
        if(!rootItm) return mig;
        for(int i = 0; i < rootItm->childCount(); i++){
            TemplateTreeItem* tableItem = rootItm->child(i);
            Q_ASSERT(tableItem->type==TemplateTreeItem::TABLE);
            for(int j = 0; j < tableItem->childCount(); j++){
                TemplateTreeItem* fieldItem = tableItem->child(j);
                Q_ASSERT(tableItem->type==TemplateTreeItem::FIELD);
                if(fieldItem->Changed()){
                    mig.AddMigration(tableItem->_table,
                                     fieldItem->_originalData.name(), fieldItem->_editableValue.toString()
                                     );
                }
            }
        }
    }
};

class TemplateTreeView : public QTreeView
{
public:
    TemplateTreeView(QWidget* parent):QTreeView(parent)
    {
        QFile f(":/css/css/treeview.css");
        if(f.open(QFile::ReadOnly)){
            QString sheet = QLatin1String(f.readAll());
            setStyleSheet(sheet);
        }else{
            qDebug() << "TemplateTables: Unable to open treeview css. Err: " << f.errorString();
        }
    }

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event)
    {
        TemplateTableModel* m = static_cast<TemplateTableModel*>(model());
        Q_ASSERT(m);
        if(event->matches(QKeySequence::Undo)){
            QModelIndex idx = m->PopChange();
            if(idx.isValid()){
                scrollTo(idx);
                selectionModel()->select(idx,QItemSelectionModel::SelectCurrent);
                viewport()->repaint();
            }
            /*
            if(m->PopChange()){
                indexAt()
                //scroll();
                viewport()->repaint();
            }*/
        }
    }
};

TemplateTables::TemplateTables(const QVector<std::pair<QString,QSqlRecord>>& records, QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    setLayout(l);

    QFormLayout* fl = new QFormLayout();
    l->addLayout(fl);
    QLineEdit* searchEdit = new QLineEdit(this);
    connect(searchEdit, &QLineEdit::textChanged, this, &TemplateTables::onTextChange);
    fl->addRow("Field/value", searchEdit);

    view = new TemplateTreeView(this);
    model = new TemplateTableModel(records, this);
    view->setModel(model);
    view->setEditTriggers(QTreeView::EditTrigger::CurrentChanged);
    view->expandAll();
    view->resizeColumnToContents(0);
    view->resizeColumnToContents(1);
    l->addWidget(view);
}

void TemplateTables::onTextChange(const QString &s)
{
    for(int r = 0; r < model->rootItm->childCount(); r++) {
        QModelIndex root = model->index(r,0,QModelIndex());
        TemplateTreeItem* rootItm = static_cast<TemplateTreeItem*>(root.internalPointer());
        for(int r0 = 0; r0 < rootItm->childCount(); r0++){
            QModelIndex r0Idx = root.child(r0,0);
            TemplateTreeItem* r0Itm = static_cast<TemplateTreeItem*>(r0Idx.internalPointer());
            view->setRowHidden(r0, root, !r0Itm->Match(s));
        }
    }
}