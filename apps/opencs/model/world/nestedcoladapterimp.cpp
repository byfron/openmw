#include "nestedcoladapterimp.hpp"

#include <components/esm/loadregn.hpp>
#include <components/esm/loadfact.hpp>

#include "idcollection.hpp"
#include "pathgrid.hpp"
#include "info.hpp"

namespace CSMWorld
{
    PathgridPointListAdapter::PathgridPointListAdapter () {}

    void PathgridPointListAdapter::addRow(Record<Pathgrid>& record, int position) const
    {
        Pathgrid pathgrid = record.get();

        ESM::Pathgrid::PointList& points = pathgrid.mPoints;

        // blank row
        ESM::Pathgrid::Point point;
        point.mX = 0;
        point.mY = 0;
        point.mZ = 0;
        point.mAutogenerated = 0;
        point.mConnectionNum = 0;
        point.mUnknown = 0;

        // inserting a point should trigger re-indexing of the edges
        //
        // FIXME: does not auto refresh edges table view
        std::vector<ESM::Pathgrid::Edge>::iterator iter = pathgrid.mEdges.begin();
        for (;iter != pathgrid.mEdges.end(); ++iter)
        {
            if ((*iter).mV0 >= position)
                (*iter).mV0++;
            if ((*iter).mV1 >= position)
                (*iter).mV1++;
        }

        points.insert(points.begin()+position, point);
        pathgrid.mData.mS2 += 1; // increment the number of points

        record.setModified (pathgrid);
    }

    void PathgridPointListAdapter::removeRow(Record<Pathgrid>& record, int rowToRemove) const
    {
        Pathgrid pathgrid = record.get();

        ESM::Pathgrid::PointList& points = pathgrid.mPoints;

        if (rowToRemove < 0 || rowToRemove >= static_cast<int> (points.size()))
            throw std::runtime_error ("index out of range");

        // deleting a point should trigger re-indexing of the edges
        // dangling edges are not allowed and hence removed
        //
        // FIXME: does not auto refresh edges table view
        std::vector<ESM::Pathgrid::Edge>::iterator iter = pathgrid.mEdges.begin();
        for (; iter != pathgrid.mEdges.end();)
        {
            if (((*iter).mV0 == rowToRemove) || ((*iter).mV1 == rowToRemove))
                iter = pathgrid.mEdges.erase(iter);
            else
            {
                if ((*iter).mV0 > rowToRemove)
                    (*iter).mV0--;

                if ((*iter).mV1 > rowToRemove)
                    (*iter).mV1--;

                ++iter;
            }
        }
        points.erase(points.begin()+rowToRemove);
        pathgrid.mData.mS2 -= 1; // decrement the number of points

        record.setModified (pathgrid);
    }

    void PathgridPointListAdapter::setTable(Record<Pathgrid>& record,
            const NestedTableWrapperBase& nestedTable) const
    {
        Pathgrid pathgrid = record.get();

        pathgrid.mPoints =
            static_cast<const PathgridPointsWrap &>(nestedTable).mRecord.mPoints;
        pathgrid.mData.mS2 =
            static_cast<const PathgridPointsWrap &>(nestedTable).mRecord.mData.mS2;
        // also update edges in case points were added/removed
        pathgrid.mEdges =
            static_cast<const PathgridPointsWrap &>(nestedTable).mRecord.mEdges;

        record.setModified (pathgrid);
    }

    NestedTableWrapperBase* PathgridPointListAdapter::table(const Record<Pathgrid>& record) const
    {
        // deleted by dtor of NestedTableStoring
        return new PathgridPointsWrap(record.get());
    }

    QVariant PathgridPointListAdapter::getData(const Record<Pathgrid>& record,
            int subRowIndex, int subColIndex) const
    {
        ESM::Pathgrid::Point point = record.get().mPoints[subRowIndex];
        switch (subColIndex)
        {
            case 0: return subRowIndex;
            case 1: return point.mX;
            case 2: return point.mY;
            case 3: return point.mZ;
            default: throw std::runtime_error("Pathgrid point subcolumn index out of range");
        }
    }

    void PathgridPointListAdapter::setData(Record<Pathgrid>& record,
            const QVariant& value, int subRowIndex, int subColIndex) const
    {
        Pathgrid pathgrid = record.get();
        ESM::Pathgrid::Point point = pathgrid.mPoints[subRowIndex];
        switch (subColIndex)
        {
            case 0: return; // return without saving
            case 1: point.mX = value.toInt(); break;
            case 2: point.mY = value.toInt(); break;
            case 3: point.mZ = value.toInt(); break;
            default: throw std::runtime_error("Pathgrid point subcolumn index out of range");
        }

        pathgrid.mPoints[subRowIndex] = point;

        record.setModified (pathgrid);
    }

    int PathgridPointListAdapter::getColumnsCount(const Record<Pathgrid>& record) const
    {
        return 4;
    }

    int PathgridPointListAdapter::getRowsCount(const Record<Pathgrid>& record) const
    {
        return static_cast<int>(record.get().mPoints.size());
    }

    PathgridEdgeListAdapter::PathgridEdgeListAdapter () {}

    // ToDo: seems to be auto-sorted in the dialog table display after insertion
    void PathgridEdgeListAdapter::addRow(Record<Pathgrid>& record, int position) const
    {
        Pathgrid pathgrid = record.get();

        ESM::Pathgrid::EdgeList& edges = pathgrid.mEdges;

        // blank row
        ESM::Pathgrid::Edge edge;
        edge.mV0 = 0;
        edge.mV1 = 0;

        // NOTE: inserting a blank edge does not really make sense, perhaps this should be a
        // logic_error exception
        //
        // Currently the code assumes that the end user to know what he/she is doing.
        // e.g. Edges come in pairs, from points a->b and b->a
        edges.insert(edges.begin()+position, edge);

        record.setModified (pathgrid);
    }

    void PathgridEdgeListAdapter::removeRow(Record<Pathgrid>& record, int rowToRemove) const
    {
        Pathgrid pathgrid = record.get();

        ESM::Pathgrid::EdgeList& edges = pathgrid.mEdges;

        if (rowToRemove < 0 || rowToRemove >= static_cast<int> (edges.size()))
            throw std::runtime_error ("index out of range");

        edges.erase(edges.begin()+rowToRemove);

        record.setModified (pathgrid);
    }

    void PathgridEdgeListAdapter::setTable(Record<Pathgrid>& record,
            const NestedTableWrapperBase& nestedTable) const
    {
        Pathgrid pathgrid = record.get();

        pathgrid.mEdges =
            static_cast<const NestedTableWrapper<ESM::Pathgrid::EdgeList> &>(nestedTable).mNestedTable;

        record.setModified (pathgrid);
    }

    NestedTableWrapperBase* PathgridEdgeListAdapter::table(const Record<Pathgrid>& record) const
    {
        // deleted by dtor of NestedTableStoring
        return new NestedTableWrapper<ESM::Pathgrid::EdgeList>(record.get().mEdges);
    }

    QVariant PathgridEdgeListAdapter::getData(const Record<Pathgrid>& record,
            int subRowIndex, int subColIndex) const
    {
        Pathgrid pathgrid = record.get();

        if (subRowIndex < 0 || subRowIndex >= static_cast<int> (pathgrid.mEdges.size()))
            throw std::runtime_error ("index out of range");

        ESM::Pathgrid::Edge edge = pathgrid.mEdges[subRowIndex];
        switch (subColIndex)
        {
            case 0: return subRowIndex;
            case 1: return edge.mV0;
            case 2: return edge.mV1;
            default: throw std::runtime_error("Pathgrid edge subcolumn index out of range");
        }
    }

    // ToDo: detect duplicates in mEdges
    void PathgridEdgeListAdapter::setData(Record<Pathgrid>& record,
            const QVariant& value, int subRowIndex, int subColIndex) const
    {
        Pathgrid pathgrid = record.get();

        if (subRowIndex < 0 || subRowIndex >= static_cast<int> (pathgrid.mEdges.size()))
            throw std::runtime_error ("index out of range");

        ESM::Pathgrid::Edge edge = pathgrid.mEdges[subRowIndex];
        switch (subColIndex)
        {
            case 0: return; // return without saving
            case 1: edge.mV0 = value.toInt(); break;
            case 2: edge.mV1 = value.toInt(); break;
            default: throw std::runtime_error("Pathgrid edge subcolumn index out of range");
        }

        pathgrid.mEdges[subRowIndex] = edge;

        record.setModified (pathgrid);
    }

    int PathgridEdgeListAdapter::getColumnsCount(const Record<Pathgrid>& record) const
    {
        return 3;
    }

    int PathgridEdgeListAdapter::getRowsCount(const Record<Pathgrid>& record) const
    {
        return static_cast<int>(record.get().mEdges.size());
    }

    FactionReactionsAdapter::FactionReactionsAdapter () {}

    void FactionReactionsAdapter::addRow(Record<ESM::Faction>& record, int position) const
    {
        ESM::Faction faction = record.get();

        std::map<std::string, int>& reactions = faction.mReactions;

        // blank row
        reactions.insert(std::make_pair("", 0));

        record.setModified (faction);
    }

    void FactionReactionsAdapter::removeRow(Record<ESM::Faction>& record, int rowToRemove) const
    {
        ESM::Faction faction = record.get();

        std::map<std::string, int>& reactions = faction.mReactions;

        if (rowToRemove < 0 || rowToRemove >= static_cast<int> (reactions.size()))
            throw std::runtime_error ("index out of range");

        // FIXME: how to ensure that the map entries correspond to table indicies?
        // WARNING: Assumed that the table view has the same order as std::map
        std::map<std::string, int>::iterator iter = reactions.begin();
        for(int i = 0; i < rowToRemove; ++i)
            iter++;
        reactions.erase(iter);

        record.setModified (faction);
    }

    void FactionReactionsAdapter::setTable(Record<ESM::Faction>& record,
            const NestedTableWrapperBase& nestedTable) const
    {
        ESM::Faction faction = record.get();

        faction.mReactions =
            static_cast<const NestedTableWrapper<std::map<std::string, int> >&>(nestedTable).mNestedTable;

        record.setModified (faction);
    }

    NestedTableWrapperBase* FactionReactionsAdapter::table(const Record<ESM::Faction>& record) const
    {
        // deleted by dtor of NestedTableStoring
        return new NestedTableWrapper<std::map<std::string, int> >(record.get().mReactions);
    }

    QVariant FactionReactionsAdapter::getData(const Record<ESM::Faction>& record,
            int subRowIndex, int subColIndex) const
    {
        ESM::Faction faction = record.get();

        std::map<std::string, int>& reactions = faction.mReactions;

        if (subRowIndex < 0 || subRowIndex >= static_cast<int> (reactions.size()))
            throw std::runtime_error ("index out of range");

        // FIXME: how to ensure that the map entries correspond to table indicies?
        // WARNING: Assumed that the table view has the same order as std::map
        std::map<std::string, int>::const_iterator iter = reactions.begin();
        for(int i = 0; i < subRowIndex; ++i)
            iter++;
        switch (subColIndex)
        {
            case 0: return QString((*iter).first.c_str());
            case 1: return (*iter).second;
            default: throw std::runtime_error("Faction reactions subcolumn index out of range");
        }
    }

    void FactionReactionsAdapter::setData(Record<ESM::Faction>& record,
            const QVariant& value, int subRowIndex, int subColIndex) const
    {
        ESM::Faction faction = record.get();

        std::map<std::string, int>& reactions = faction.mReactions;

        if (subRowIndex < 0 || subRowIndex >= static_cast<int> (reactions.size()))
            throw std::runtime_error ("index out of range");

        // FIXME: how to ensure that the map entries correspond to table indicies?
        // WARNING: Assumed that the table view has the same order as std::map
        std::map<std::string, int>::iterator iter = reactions.begin();
        for(int i = 0; i < subRowIndex; ++i)
            iter++;

        std::string factionId = (*iter).first;
        int reaction = (*iter).second;

        switch (subColIndex)
        {
            case 0:
            {
                reactions.erase(iter);
                reactions.insert(std::make_pair(value.toString().toUtf8().constData(), reaction));
                break;
            }
            case 1:
            {
                reactions[factionId] = value.toInt();
                break;
            }
            default: throw std::runtime_error("Faction reactions subcolumn index out of range");
        }

        record.setModified (faction);
    }

    int FactionReactionsAdapter::getColumnsCount(const Record<ESM::Faction>& record) const
    {
        return 2;
    }

    int FactionReactionsAdapter::getRowsCount(const Record<ESM::Faction>& record) const
    {
        return static_cast<int>(record.get().mReactions.size());
    }

    RegionSoundListAdapter::RegionSoundListAdapter () {}

    void RegionSoundListAdapter::addRow(Record<ESM::Region>& record, int position) const
    {
        ESM::Region region = record.get();

        std::vector<ESM::Region::SoundRef>& soundList = region.mSoundList;

        // blank row
        ESM::Region::SoundRef soundRef;
        soundRef.mSound.assign("");
        soundRef.mChance = 0;

        soundList.insert(soundList.begin()+position, soundRef);

        record.setModified (region);
    }

    void RegionSoundListAdapter::removeRow(Record<ESM::Region>& record, int rowToRemove) const
    {
        ESM::Region region = record.get();

        std::vector<ESM::Region::SoundRef>& soundList = region.mSoundList;

        if (rowToRemove < 0 || rowToRemove >= static_cast<int> (soundList.size()))
            throw std::runtime_error ("index out of range");

        soundList.erase(soundList.begin()+rowToRemove);

        record.setModified (region);
    }

    void RegionSoundListAdapter::setTable(Record<ESM::Region>& record,
            const NestedTableWrapperBase& nestedTable) const
    {
        ESM::Region region = record.get();

        region.mSoundList =
            static_cast<const NestedTableWrapper<std::vector<ESM::Region::SoundRef> >&>(nestedTable).mNestedTable;

        record.setModified (region);
    }

    NestedTableWrapperBase* RegionSoundListAdapter::table(const Record<ESM::Region>& record) const
    {
        // deleted by dtor of NestedTableStoring
        return new NestedTableWrapper<std::vector<ESM::Region::SoundRef> >(record.get().mSoundList);
    }

    QVariant RegionSoundListAdapter::getData(const Record<ESM::Region>& record,
            int subRowIndex, int subColIndex) const
    {
        ESM::Region region = record.get();

        std::vector<ESM::Region::SoundRef>& soundList = region.mSoundList;

        if (subRowIndex < 0 || subRowIndex >= static_cast<int> (soundList.size()))
            throw std::runtime_error ("index out of range");

        ESM::Region::SoundRef soundRef = soundList[subRowIndex];
        switch (subColIndex)
        {
            case 0: return QString(soundRef.mSound.toString().c_str());
            case 1: return soundRef.mChance;
            default: throw std::runtime_error("Region sounds subcolumn index out of range");
        }
    }

    void RegionSoundListAdapter::setData(Record<ESM::Region>& record,
            const QVariant& value, int subRowIndex, int subColIndex) const
    {
        ESM::Region region = record.get();

        std::vector<ESM::Region::SoundRef>& soundList = region.mSoundList;

        if (subRowIndex < 0 || subRowIndex >= static_cast<int> (soundList.size()))
            throw std::runtime_error ("index out of range");

        ESM::Region::SoundRef soundRef = soundList[subRowIndex];
        switch (subColIndex)
        {
            case 0: soundRef.mSound.assign(value.toString().toUtf8().constData()); break;
            case 1: soundRef.mChance = static_cast<unsigned char>(value.toInt()); break;
            default: throw std::runtime_error("Region sounds subcolumn index out of range");
        }

        region.mSoundList[subRowIndex] = soundRef;

        record.setModified (region);
    }

    int RegionSoundListAdapter::getColumnsCount(const Record<ESM::Region>& record) const
    {
        return 2;
    }

    int RegionSoundListAdapter::getRowsCount(const Record<ESM::Region>& record) const
    {
        return static_cast<int>(record.get().mSoundList.size());
    }

    InfoListAdapter::InfoListAdapter () {}

    void InfoListAdapter::addRow(Record<Info>& record, int position) const
    {
        throw std::logic_error ("cannot add a row to a fixed table");
    }

    void InfoListAdapter::removeRow(Record<Info>& record, int rowToRemove) const
    {
        throw std::logic_error ("cannot add a row to a fixed table");
    }

    void InfoListAdapter::setTable(Record<Info>& record,
            const NestedTableWrapperBase& nestedTable) const
    {
        throw std::logic_error ("table operation not supported");
    }

    NestedTableWrapperBase* InfoListAdapter::table(const Record<Info>& record) const
    {
        throw std::logic_error ("table operation not supported");
    }

    QVariant InfoListAdapter::getData(const Record<Info>& record,
            int subRowIndex, int subColIndex) const
    {
        Info info = record.get();

        if (subColIndex == 0)
            return QString(info.mResultScript.c_str());
        else
            throw std::runtime_error("Trying to access non-existing column in the nested table!");
    }

    void InfoListAdapter::setData(Record<Info>& record,
            const QVariant& value, int subRowIndex, int subColIndex) const
    {
        Info info = record.get();

        if (subColIndex == 0)
            info.mResultScript = value.toString().toStdString();
        else
            throw std::runtime_error("Trying to access non-existing column in the nested table!");

        record.setModified (info);
    }

    int InfoListAdapter::getColumnsCount(const Record<Info>& record) const
    {
        return 1;
    }

    int InfoListAdapter::getRowsCount(const Record<Info>& record) const
    {
        return 1; // fixed at size 1
    }
}
