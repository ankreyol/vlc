/*****************************************************************************
 * mediacenter_model.cpp : Manage mediacenter model
 ****************************************************************************
 * Copyright (C) 2006-2011 the VideoLAN team
 * $Id$
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Ilkka Ollakkka <ileoo (at) videolan dot org>
 *          Jakob Leben <jleben@videolan.org>
 *          Maël Kervella <dev@maelkervella.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "qt.hpp"
#include "components/playlist/mediacenter_model.hpp"
#include "input_manager.hpp"                            /* THEMIM */
#include "util/qt_dirs.hpp"
#include "recents.hpp"                                  /* Open:: */

#include <vlc_intf_strings.h>                           /* I_DIR */
#include <vlc_url.h>

#include "sorting.h"

#include <assert.h>
#include <QFont>
#include <QAction>
#include <QStack>

/*************************************************************************
 * Playlist model implementation
 *************************************************************************/

MCModel::MCModel( playlist_t *_p_playlist,  /* THEPL */
                  intf_thread_t *_p_intf,   /* main Qt p_intf */
                  playlist_item_t * p_root,
                  QObject *parent )         /* Basic Qt parent */
                  : VLCModel( _p_intf, parent )
{
    p_playlist        = _p_playlist;

    rootItem          = NULL; /* PLItem rootItem, will be set in rebuild( ) */
    latestSearch      = QString();

    rebuild( p_root );
    DCONNECT( THEMIM->getIM(), metaChanged( input_item_t *),
              this, processInputItemUpdate( input_item_t *) );
    DCONNECT( THEMIM, inputChanged( bool ),
              this, processInputItemUpdate( ) );
    CONNECT( THEMIM, playlistItemAppended( int, int ),
             this, processItemAppend( int, int ) );
    CONNECT( THEMIM, playlistItemRemoved( int ),
             this, processItemRemoval( int ) );

}

MCModel::~MCModel()
{
    delete rootItem;
}

Qt::DropActions MCModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags MCModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags( index );

    const MCItem *item = index.isValid() ? getItem( index ) : rootItem;

    if( canEdit() )
    {
        vlc_playlist_locker pl_lock ( THEPL );

        playlist_item_t *plItem =
            playlist_ItemGetById( p_playlist, item->i_playlist_id );

        if ( plItem && ( plItem->i_children > -1 ) )
            flags |= Qt::ItemIsDropEnabled;
    }
    flags |= Qt::ItemIsDragEnabled;

    return flags;
}

QStringList MCModel::mimeTypes() const
{
    QStringList types;
    types << "vlc/qt-input-items";
    return types;
}

bool modelIndexLessThen( const QModelIndex &i1, const QModelIndex &i2 )
{
    if( !i1.isValid() || !i2.isValid() ) return false;
    MCItem *item1 = static_cast<MCItem*>( i1.internalPointer() );
    MCItem *item2 = static_cast<MCItem*>( i2.internalPointer() );
    if( item1->hasSameParent( item2 ) ) return i1.row() < i2.row();
    else return *item1 < *item2;
}

QMimeData *MCModel::mimeData( const QModelIndexList &indexes ) const
{
    PlMimeData *plMimeData = new PlMimeData();
    QModelIndexList list;

    foreach( const QModelIndex &index, indexes ) {
        if( index.isValid() && index.column() == 0 )
            list.append(index);
    }

    qSort(list.begin(), list.end(), modelIndexLessThen);

    MCItem *item = NULL;
    foreach( const QModelIndex &index, list ) {
        if( item )
        {
            MCItem *testee = getItem( index );
            while( testee->plitem_parent() )
            {
                if( testee->plitem_parent() == item ||
                    testee->plitem_parent() == item->plitem_parent() ) break;
                testee = testee->plitem_parent();
            }
            if( testee->plitem_parent() == item ) continue;
            item = getItem( index );
        }
        else
            item = getItem( index );

        plMimeData->appendItem( static_cast<MCItem*>(item)->inputItem() );
    }

    return plMimeData;
}

/* Drop operation */
bool MCModel::dropMimeData( const QMimeData *data, Qt::DropAction action,
        int row, int, const QModelIndex &parent )
{
    bool copy = action == Qt::CopyAction;
    if( !copy && action != Qt::MoveAction )
        return true;

    const PlMimeData *plMimeData = qobject_cast<const PlMimeData*>( data );
    if( plMimeData )
    {
        if( copy )
            dropAppendCopy( plMimeData, getItem( parent ), row );
        else
            dropMove( plMimeData, getItem( parent ), row );
    }
    return true;
}

void MCModel::dropAppendCopy( const PlMimeData *plMimeData, MCItem *target, int pos )
{
    vlc_playlist_locker pl_lock ( THEPL );

    playlist_item_t *p_parent =
        playlist_ItemGetByInput( p_playlist, target->inputItem() );
    if( !p_parent ) return;

    if( pos == -1 ) pos = PLAYLIST_END;

    QList<input_item_t*> inputItems = plMimeData->inputItems();

    foreach( input_item_t* p_input, inputItems )
    {
        playlist_item_t *p_item = playlist_ItemGetByInput( p_playlist, p_input );
        if( !p_item ) continue;
        pos = playlist_NodeAddCopy( p_playlist, p_item, p_parent, pos );
    }
}

void MCModel::dropMove( const PlMimeData * plMimeData, MCItem *target, int row )
{
    QList<input_item_t*> inputItems = plMimeData->inputItems();
    QList<MCItem*> model_items;
    playlist_item_t **pp_items;
    pp_items = (playlist_item_t **)
               calloc( inputItems.count(), sizeof( playlist_item_t* ) );
    if ( !pp_items ) return;

    int model_pos;

    {
        vlc_playlist_locker pl_lock ( THEPL );

        playlist_item_t *p_parent =
            playlist_ItemGetByInput( p_playlist, target->inputItem() );

        if( !p_parent || row > p_parent->i_children )
        {
            free( pp_items );
            return;
        }

        int new_pos = model_pos = row == -1 ? p_parent->i_children : row;
        int i = 0;

        foreach( input_item_t *p_input, inputItems )
        {
            playlist_item_t *p_item = playlist_ItemGetByInput( p_playlist, p_input );
            if( !p_item ) continue;

            MCItem *item = findByInputLocked( rootItem, p_input );
            if( !item ) continue;

            /* Better not try to move a node into itself.
               Abort the whole operation in that case,
               because it is ambiguous. */
            MCItem *climber = target;
            while( climber )
            {
                if( climber == item )
                {
                    free( pp_items );
                    return;
                }
                climber = climber->plitem_parent();
            }

            if( item->plitem_parent() == target &&
                target->plitem_children.indexOf( item ) < new_pos )
                model_pos--;

            model_items.append( item );
            pp_items[i] = p_item;
            i++;
        }

        if( model_items.isEmpty() )
        {
            free( pp_items );
            return;
        }

        playlist_TreeMoveMany( p_playlist, i, pp_items, p_parent, new_pos );
    }

    foreach( MCItem *item, model_items )
        takeItem( item );

    insertChildren( target, model_items, model_pos );
    free( pp_items );
}

void MCModel::activateItem( const QModelIndex &index )
{
    assert( index.isValid() );
    const MCItem *item = getItem( index );
    assert( item );

    vlc_playlist_locker pl_lock( THEPL );

    playlist_item_t *p_item = playlist_ItemGetById( p_playlist, item->i_playlist_id );
    activateItem( p_item );
}

/* Convenient overloaded private version of activateItem
 * Must be entered with PL lock */
void MCModel::activateItem( playlist_item_t *p_item )
{
    if( !p_item ) return;
    playlist_item_t *p_parent = p_item;
    while( p_parent )
    {
        if( p_parent->i_id == rootItem->id() ) break;
        p_parent = p_parent->p_parent;
    }
    if( p_parent )
        playlist_ViewPlay( p_playlist, p_parent, p_item );
}

/****************** Base model mandatory implementations *****************/
QVariant MCModel::data( const QModelIndex &index, const int role ) const
{
    if( !index.isValid() )
        return QVariant();

    switch( role )
    {

        case Qt::FontRole:
            return customFont;

        case Qt::DisplayRole:
        {
            MCItem *item = getItem( index );
            int metadata = columnToMeta( index.column() );
            if( metadata == COLUMN_END )
                return QVariant();

            QString returninfo;
            if( metadata == COLUMN_NUMBER )
            {
                returninfo = QString::number( index.row() + 1 );
            }
            else if( metadata == COLUMN_COVER )
            {
                QString artUrl;
                artUrl = InputManager::decodeArtURL( item->inputItem() );
                if( artUrl.isEmpty() )
                {
                    for( int i = 0; i < item->childCount(); i++ )
                    {
                        artUrl = InputManager::decodeArtURL( item->child( i )->inputItem() );
                        if( !artUrl.isEmpty() )
                            break;
                    }
                }
                return artUrl;
            }
            else
            {
                char *psz = psz_column_meta( item->inputItem(), metadata );
                returninfo = qfu( psz );
                free( psz );
            }

            return QVariant( returninfo );
        }

        case Qt::DecorationRole:
        {
            switch( columnToMeta(index.column()) )
            {
                case COLUMN_TITLE:
                {
                    MCItem *item = getItem( index );
                    /* Used to segfault here because i_type wasn't always initialized */
                    enum input_item_type_e idx = item->inputItem()->i_type;
                    if( item->inputItem()->b_net && item->inputItem()->i_type == ITEM_TYPE_FILE )
                        idx = ITEM_TYPE_STREAM;
                    return QVariant( icons[idx] );
                }
                case COLUMN_COVER:
                    /* !warn: changes tree item line height. Otherwise, override
                     * delegate's sizehint */
                    return getArtPixmap( index, QSize(16,16) );
                default:
                    break;
            }
            break;
        }

        case Qt::BackgroundRole:
            if( isCurrent( index ) )
                return QVariant( QBrush( Qt::gray ) );
            break;

        case CURRENT_ITEM_ROLE:
            return QVariant( isCurrent( index ) );

        case CURRENT_ITEM_CHILD_ROLE:
            return QVariant( isParent( index, currentIndex() ) );

        case LEAF_NODE_ROLE:
            return QVariant( isLeaf( index ) );

        default:
            break;
    }

    return QVariant();
}

bool MCModel::setData( const QModelIndex &index, const QVariant & value, int role )
{
    switch( role )
    {
    case Qt::FontRole:
        customFont = value.value<QFont>();
        return true;
    default:
        return VLCModel::setData( index, value, role );
    }
}

/* Seek from current index toward the top and see if index is one of parent nodes */
bool MCModel::isParent( const QModelIndex &index, const QModelIndex &current ) const
{
    if( !index.isValid() )
        return false;

    if( index == current )
        return true;

    if( !current.isValid() || !current.parent().isValid() )
        return false;

    return isParent( index, current.parent() );
}

bool MCModel::isLeaf( const QModelIndex &index ) const
{
    bool b_isLeaf = false;

    vlc_playlist_locker pl_lock ( THEPL );

    playlist_item_t *plItem =
        playlist_ItemGetById( p_playlist, itemId( index ) );

    if( plItem )
        b_isLeaf = plItem->i_children == -1;

    return b_isLeaf;
}

MCItem* MCModel::getItem( const QModelIndex & index ) const
{
    MCItem *item = static_cast<MCItem *>( VLCModel::getItem( index ) );
    if ( item == NULL ) item = rootItem;
    return item;
}

QModelIndex MCModel::index( const int row, const int column, const QModelIndex &parent )
                  const
{
    MCItem *parentItem = parent.isValid() ? getItem( parent ) : rootItem;

    MCItem *childItem = static_cast<MCItem*>(parentItem->child( row ));
    if( childItem )
        return createIndex( row, column, childItem );
    else
        return QModelIndex();
}

QModelIndex MCModel::indexByPLID( const int i_plid, const int c ) const
{
    return index( findByPLId( rootItem, i_plid ), c );
}

QModelIndex MCModel::indexByInputItem( const input_item_t *item, const int c ) const
{
    return index( findByInput( rootItem, item ), c );
}

QModelIndex MCModel::rootIndex() const
{
    return index( findByPLId( rootItem, rootItem->id() ), 0 );
}

bool MCModel::isTree() const
{
    return ( ( rootItem && rootItem->id() != p_playlist->p_playing->i_id )
             || var_InheritBool( p_intf, "playlist-tree" ) );
}

/* Return the index of a given item */
QModelIndex MCModel::index( MCItem *item, int column ) const
{
    if( !item ) return QModelIndex();
    MCItem *parent = item->plitem_parent();
    if( parent )
        return createIndex( parent->lastIndexOf( item ),
                            column, item );
    return QModelIndex();
}

QModelIndex MCModel::currentIndex() const
{
    input_thread_t *p_input_thread = THEMIM->getInput();
    if( !p_input_thread ) return QModelIndex();
    MCItem *item = findByInput( rootItem, input_GetItem( p_input_thread ) );
    return index( item, 0 );
}

QModelIndex MCModel::parent( const QModelIndex &index ) const
{
    if( !index.isValid() ) return QModelIndex();

    MCItem *childItem = getItem( index );
    if( !childItem )
    {
        msg_Err( p_playlist, "Item not found" );
        return QModelIndex();
    }

    MCItem *parentItem = static_cast<MCItem*>(childItem->plitem_parent());
    if( !parentItem || parentItem == rootItem ) return QModelIndex();
    if( !parentItem->plitem_parent() )
    {
        msg_Err( p_playlist, "No parent found, trying row 0. Please report this" );
        return createIndex( 0, 0, parentItem );
    }
    return createIndex(parentItem->row(), 0, parentItem);
}

int MCModel::rowCount( const QModelIndex &parent ) const
{
    MCItem *parentItem = parent.isValid() ? getItem( parent ) : rootItem;
    return parentItem->childCount();
}

/************************* Lookups *****************************/
MCItem *MCModel::findByPLId( MCItem *root, int i_id ) const
{
    if( !root ) return NULL;

    if( root->id() == i_id )
        return root;

    /* traverse the tree (in depth first) iteratively to avoid stack overflow */

    struct RemainingChildren {
        QList<MCItem *>::const_iterator next;
        QList<MCItem *>::const_iterator end;
    };

    QStack<RemainingChildren> stack;
    if( root->childCount() )
        stack.push( {root->plitem_children.cbegin(), root->plitem_children.cend()} );

    while ( !stack.isEmpty() )
    {
        RemainingChildren &remainingChildren = stack.top();

        MCItem *item = static_cast<MCItem *>( *remainingChildren.next );
        if( item->id() == i_id )
            return item;

        if( ++remainingChildren.next == remainingChildren.end )
            /* there are no more children at this depth level */
            stack.pop();

        if( item->childCount() )
            stack.push( {item->plitem_children.cbegin(), item->plitem_children.cend()} );
    }
    return NULL;
}

MCItem *MCModel::findByInput( MCItem *root, const input_item_t *input ) const
{
    int i_id;
    {
        playlist_item_t *item;

        vlc_playlist_locker pl_lock ( THEPL );
        item = playlist_ItemGetByInput( THEPL, input );
        if( item == NULL )
            return NULL;
        i_id = item->i_id;
    }
    return findByPLId( root, i_id );
}

MCItem *MCModel::findByInputLocked( MCItem *root, const input_item_t *input ) const
{
    PL_ASSERT_LOCKED;

    playlist_item_t* item = playlist_ItemGetByInput( THEPL, input );
    if( item == NULL )
        return NULL;
    return findByPLId( root, item->i_id );
}

MCModel::pl_nodetype MCModel::getPLRootType() const
{
    vlc_playlist_locker pl_lock ( THEPL );

    /* can't rely on rootitem as it depends on view / rebuild() */
    MCItem *plitem = rootItem;
    while( plitem->plitem_parent() )plitem = plitem->plitem_parent();

    if( plitem->id() == p_playlist->p_playing->i_id )
        return ROOTTYPE_CURRENT_PLAYING;

    return ROOTTYPE_OTHER;
}

bool MCModel::canEdit() const
{
    return ( getPLRootType() != ROOTTYPE_OTHER );
}

/************************* Updates handling *****************************/

/**** Events processing ****/
void MCModel::processInputItemUpdate( )
{
    input_thread_t *p_input = THEMIM->getInput();
    if( !p_input ) return;

    MCItem *item = findByInput( rootItem, input_GetItem( p_input ) );
    if( item ) emit currentIndexChanged( index( item, 0 ) );

    processInputItemUpdate( input_GetItem( p_input ) );
}

void MCModel::processInputItemUpdate( input_item_t *p_item )
{
    if( !p_item ) return;
    MCItem *item = findByInput( rootItem, p_item );
    if( item )
        updateTreeItem( item );
}

void MCModel::processItemRemoval( int i_pl_itemid )
{
    if( i_pl_itemid <= 0 ) return;
    removeItem( findByPLId( rootItem, i_pl_itemid ) );
}

void MCModel::processItemAppend( int i_pl_itemid, int i_pl_itemidparent )
{
    playlist_item_t *p_item = NULL;
    MCItem *newItem = NULL;
    int pos;

    /* Find the Parent */
    MCItem *nodeParentItem = findByPLId( rootItem, i_pl_itemidparent );
    if( !nodeParentItem ) return;

    /* Search for an already matching children */
    foreach( MCItem *existing, nodeParentItem->plitem_children )
        if( existing->id() == i_pl_itemid ) return;

    /* Find the child */
    {
        vlc_playlist_locker pl_lock ( THEPL );

        p_item = playlist_ItemGetById( p_playlist, i_pl_itemid );
        if( !p_item || p_item->i_flags & PLAYLIST_DBL_FLAG )
            return;

        for( pos = p_item->p_parent->i_children - 1; pos >= 0; pos-- )
            if( p_item->p_parent->pp_children[pos] == p_item ) break;

        newItem = new MCItem( p_item, nodeParentItem );
    }

    /* We insert the newItem (children) inside the parent */
    beginInsertRows( index( nodeParentItem, 0 ), pos, pos );
    nodeParentItem->insertChild( newItem, pos );
    endInsertRows();
    if ( newItem->inputItem() == THEMIM->currentInputItem() )
        emit currentIndexChanged( index( newItem, 0 ) );

    if( latestSearch.isEmpty() ) return;
    filter( latestSearch, index( rootItem, 0), false /*FIXME*/ );
}

void MCModel::rebuild( playlist_item_t *p_root )
{
    beginResetModel();

    {
        vlc_playlist_locker pl_lock ( THEPL );

        if( rootItem ) rootItem->clearChildren();
        if( p_root ) // Can be NULL
        {
            if ( rootItem ) delete rootItem;
            rootItem = new MCItem( p_root, NULL );
        }
        assert( rootItem );
        /* Recreate from root */
        updateChildren( rootItem );
    }

    /* And signal the view */
    endResetModel();
    if( p_root ) emit rootIndexChanged();
}

void MCModel::takeItem( MCItem *item )
{
    assert( item );
    MCItem *parent = static_cast<MCItem*>(item->plitem_parent());
    assert( parent );
    int i_index = parent->indexOf( item );

    beginRemoveRows( index( parent, 0 ), i_index, i_index );
    parent->takeChildAt( i_index );
    endRemoveRows();
}

void MCModel::insertChildren( MCItem *node, QList<MCItem*>& items, int i_pos )
{
    assert( node );
    int count = items.count();
    if( !count ) return;
    beginInsertRows( index( node, 0 ), i_pos, i_pos + count - 1 );
    for( int i = 0; i < count; i++ )
    {
        node->plitem_children.insert( i_pos + i, items[i] );
        items[i]->parentItem = node;
    }
    endInsertRows();
}

void MCModel::removeItem( MCItem *item )
{
    if( !item ) return;

    if( item->plitem_parent() ) {
        int i = item->plitem_parent()->indexOf( item );
        beginRemoveRows( index( static_cast<MCItem*>(item->plitem_parent()), 0), i, i );
        item->plitem_parent()->plitem_children.removeAt(i);
        delete item;
        endRemoveRows();
    }
    else delete item;

    if(item == rootItem)
    {
        rootItem = NULL;
        rebuild( p_playlist->p_playing );
    }
}

/* This function must be entered WITH the playlist lock */
void MCModel::updateChildren( MCItem *root )
{
    playlist_item_t *p_node = playlist_ItemGetById( p_playlist, root->id() );
    updateChildren( p_node, root );
}

/* This function must be entered WITH the playlist lock */
void MCModel::updateChildren( playlist_item_t *p_node, MCItem *root )
{
    for( int i = 0; i < p_node->i_children ; i++ )
    {
        if( p_node->pp_children[i]->i_flags & PLAYLIST_DBL_FLAG ) continue;
        MCItem *newItem = new MCItem( p_node->pp_children[i], root );
        root->appendChild( newItem );
        if( p_node->pp_children[i]->i_children != -1 )
            updateChildren( p_node->pp_children[i], newItem );
    }
}

/* Function doesn't need playlist-lock, as we don't touch playlist_item_t stuff here*/
void MCModel::updateTreeItem( MCItem *item )
{
    if( !item ) return;
    emit dataChanged( index( item, 0 ) , index( item, columnCount( QModelIndex() ) - 1 ) );
}

/************************* Actions ******************************/

/**
 * Deletion, don't delete items childrens if item is going to be
 * delete allready, so we remove childrens from selection-list.
 */
void MCModel::doDelete( QModelIndexList selected )
{
    if( !canEdit() ) return;

    while( !selected.isEmpty() )
    {
        QModelIndex index = selected[0];
        selected.removeAt( 0 );

        if( index.column() != 0 ) continue;

        MCItem *item = getItem( index );
        if( item->childCount() )
            recurseDelete( item->plitem_children, &selected );

        PL_LOCK;
        playlist_item_t *p_root = playlist_ItemGetById( p_playlist,
                                                        item->id() );
        if( p_root != NULL )
            playlist_NodeDelete( p_playlist, p_root );
        PL_UNLOCK;

        if( p_root != NULL )
            removeItem( item );
    }
}

void MCModel::recurseDelete( QList<MCItem*> children, QModelIndexList *fullList )
{
    for( int i = children.count() - 1; i >= 0 ; i-- )
    {
        MCItem *item = static_cast<MCItem *>(children[i]);
        if( item->childCount() )
            recurseDelete( item->plitem_children, fullList );
        fullList->removeAll( index( item, 0 ) );
    }
}

/******* Volume III: Sorting and searching ********/
void MCModel::sort( const int column, Qt::SortOrder order )
{
    sort( QModelIndex(), indexByPLID( rootItem->id(), 0 ) , column, order );
}

void MCModel::sort( QModelIndex caller, QModelIndex rootIndex, const int column, Qt::SortOrder order )
{
    msg_Dbg( p_intf, "Sorting by column %i, order %i", column, order );

    int meta = columnToMeta( column );
    if( meta == COLUMN_END || meta == COLUMN_COVER ) return;

    MCItem *item = ( rootIndex.isValid() ) ? getItem( rootIndex )
                                           : rootItem;
    if( !item ) return;

    input_item_t* p_caller_item = caller.isValid()
        ? static_cast<MCItem*>( caller.internalPointer() )->inputItem()
        : NULL;

    int i_root_id = item->id();

    QModelIndex qIndex = index( item, 0 );
    int count = item->childCount();
    if( count )
    {
        beginRemoveRows( qIndex, 0, count - 1 );
        item->clearChildren();
        endRemoveRows( );
    }

    {
        vlc_playlist_locker pl_lock ( THEPL );

        playlist_item_t *p_root = playlist_ItemGetById( p_playlist,
                                                        i_root_id );
        if( p_root )
        {
            playlist_RecursiveNodeSort( p_playlist, p_root,
                                        i_column_sorting( meta ),
                                        order == Qt::AscendingOrder ?
                                            ORDER_NORMAL : ORDER_REVERSE );
        }

        if( count )
        {
            beginInsertRows( qIndex, 0, count - 1 );
            updateChildren( item );
            endInsertRows( );
        }
    }

    /* if we have popup item, try to make sure that you keep that item visible */
    if( p_caller_item )
    {
        QModelIndex idx = indexByInputItem( p_caller_item, 0 );

        emit currentIndexChanged( idx );
    }
    else if( currentIndex().isValid() )
        emit currentIndexChanged( currentIndex() );
}

void MCModel::filter( const QString& search_text, const QModelIndex & idx, bool b_recursive )
{
    latestSearch = search_text;

    /** \todo Fire the search with a small delay ? */
    {
        vlc_playlist_locker pl_lock ( THEPL );

        playlist_item_t *p_root = playlist_ItemGetById( p_playlist,
                                                        itemId( idx ) );
        assert( p_root );
        playlist_LiveSearchUpdate( p_playlist, p_root, qtu( search_text ),
                                   b_recursive );
        if( idx.isValid() )
        {
            MCItem *searchRoot = getItem( idx );

            beginRemoveRows( idx, 0, searchRoot->childCount() - 1 );
            searchRoot->clearChildren();
            endRemoveRows();

            beginInsertRows( idx, 0, searchRoot->childCount() - 1 );
            updateChildren( searchRoot ); // The PL_LOCK is needed here
            endInsertRows();

            return;
        }
    }

    rebuild();
}

void MCModel::removeAll()
{
    if( rowCount() < 1 ) return;

    QModelIndexList l;
    for( int i = 0; i < rowCount(); i++)
    {
        QModelIndex indexrecord = index( i, 0, QModelIndex() );
        l.append( indexrecord );
    }
    doDelete(l);
}

void MCModel::createNode( QModelIndex index, QString name )
{
    if( name.isEmpty() )
        return;

    vlc_playlist_locker pl_lock ( THEPL );

    index = index.parent();
    if ( !index.isValid() ) index = rootIndex();
    playlist_item_t *p_item = playlist_ItemGetById( p_playlist, itemId( index ) );
    if( p_item )
        playlist_NodeCreate( p_playlist, qtu( name ), p_item, PLAYLIST_END, 0 );
}

void MCModel::renameNode( QModelIndex index, QString name )
{
    if( name.isEmpty() || !index.isValid() ) return;

    vlc_playlist_locker pl_lock ( THEPL );

    if ( !index.isValid() ) index = rootIndex();
    input_item_t* p_input = this->getInputItem( index );
    input_item_SetName( p_input, qtu( name ) );
    playlist_t *p_playlist = THEPL;
    input_item_WriteMeta( VLC_OBJECT(p_playlist), p_input );
}

bool MCModel::action( QAction *action, const QModelIndexList &indexes )
{
    QModelIndex index;
    actionsContainerType a = action->data().value<actionsContainerType>();

    switch ( a.action )
    {

    case ACTION_PLAY:
        if ( !indexes.empty() && indexes.first().isValid() )
        {
            if( isCurrent( indexes.first() ) )
                playlist_Resume(THEPL);
            else
                activateItem( indexes.first() );
            return true;
        }
        break;

    case ACTION_PAUSE:
        if ( !indexes.empty() && indexes.first().isValid() )
        {
            playlist_Pause(THEPL);
            return true;
        }
        break;

    case ACTION_ADDTOPLAYLIST:
    {
        vlc_playlist_locker pl_lock ( THEPL );

        foreach( const QModelIndex &currentIndex, indexes )
        {
            playlist_item_t *p_item = playlist_ItemGetById( THEPL, itemId( currentIndex ) );
            if( !p_item ) continue;

            playlist_NodeAddCopy( THEPL, p_item,
                                  THEPL->p_playing,
                                  PLAYLIST_END );
        }
        return true;
    }

    case ACTION_REMOVE:
        doDelete( indexes );
        return true;

    case ACTION_SORT:
        if ( !indexes.empty() )
            index = indexes.first();

        sort( index, rootIndex(),
              a.column > 0 ? a.column - 1 : -a.column - 1,
              a.column > 0 ? Qt::AscendingOrder : Qt::DescendingOrder );
        return true;

    case ACTION_CLEAR:
        removeAll();
        return true;

    case ACTION_ENQUEUEFILE:
        foreach( const QString &uri, a.uris )
            Open::openMRL( p_intf, uri.toLatin1().constData(),
                           false );
        return true;

    case ACTION_ENQUEUEDIR:
        if( a.uris.isEmpty() ) break;

        Open::openMRL( p_intf, a.uris.first().toLatin1().constData(),
                       false );

        return true;

    case ACTION_ENQUEUEGENERIC:
        foreach( const QString &uri, a.uris )
        {
            QStringList options = a.options.split( " :" );
            Open::openMRLwithOptions( p_intf, uri, &options, false );
        }
        return true;

    default:
        break;
    }
    return false;
}

bool MCModel::isSupportedAction( actions action, const QModelIndex &index ) const
{
    MCItem const* item = VLCModel::getItem( index );

    switch ( action )
    {
    case ACTION_ADDTOPLAYLIST:
        /* Only if we are not already in Current Playing */
        return getPLRootType() != ROOTTYPE_CURRENT_PLAYING;
    case ACTION_SORT:
        return rowCount();
    case ACTION_PLAY:
    {
        if( !item )
            return false;

        {
            vlc_playlist_locker pl_lock ( THEPL );

            if( playlist_Status( THEPL ) != PLAYLIST_RUNNING )
                return true;
        }

        return !isCurrent( index );
    }
    case ACTION_PAUSE:
    {
        if( !isCurrent( index ) )
            return false;

        vlc_playlist_locker pl_lock ( THEPL );

        return playlist_Status( THEPL ) == PLAYLIST_RUNNING;
    }
    case ACTION_STREAM:
    case ACTION_SAVE:
    case ACTION_INFO:
        return item;
    case ACTION_REMOVE:
        return item && !item->readOnly();
    case ACTION_EXPLORE:
    {
        if( !item )
            return false;

        char*  psz_path = vlc_uri2path( qtu( item->getURI() ) );
        free(  psz_path );
        return psz_path != NULL;
    }
    case ACTION_CREATENODE:
            return canEdit() && isTree() && ( !item || !item->readOnly() );
    case ACTION_RENAMENODE:
            return item && !isLeaf( index ) && !item->readOnly();
    case ACTION_CLEAR:
            return canEdit() && rowCount();
    case ACTION_ENQUEUEFILE:
    case ACTION_ENQUEUEDIR:
    case ACTION_ENQUEUEGENERIC:
        return canEdit();
    case ACTION_SAVETOPLAYLIST:
        return getPLRootType() == ROOTTYPE_CURRENT_PLAYING && rowCount();
    default:
        return false;
    }
    return false;
}

/******************* Drag and Drop helper class ******************/
PlMimeData::~PlMimeData()
{
    foreach( input_item_t *p_item, _inputItems )
        input_item_Release( p_item );
}

void PlMimeData::appendItem( input_item_t *p_item )
{
    input_item_Hold( p_item );
    _inputItems.append( p_item );
}

QList<input_item_t*> PlMimeData::inputItems() const
{
    return _inputItems;
}

QStringList PlMimeData::formats () const
{
    QStringList fmts;
    fmts << "vlc/qt-input-items";
    return fmts;
}