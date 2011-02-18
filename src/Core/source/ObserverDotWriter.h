/****************************************************************************
**
** Copyright (c) 2009-2011, Jaco Naude
**
** This file is part of Qtilities which is released under the following
** licensing options.
**
** Option 1: Open Source
** Under this license Qtilities is free software: you can
** redistribute it and/or modify it under the terms of the GNU General
** Public License as published by the Free Software Foundation, either
** version 3 of the License, or (at your option) any later version.
**
** Qtilities is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Qtilities. If not, see http://www.gnu.org/licenses/.
**
** Option 2: Commercial
** Alternatively, this library is also released under a commercial license
** that allows the development of closed source proprietary applications
** without restrictions on licensing. For more information on this option,
** please see the project website's licensing page:
** http://www.qtilities.org/licensing.html
**
** If you are unsure which license is appropriate for your use, please
** contact support@qtilities.org.
**
****************************************************************************/

#ifndef OBSERVERDOTGRAPH_H
#define OBSERVERDOTGRAPH_H

#include "QtilitiesCore_global.h"

#include <QObject>
#include <QString>
#include <QHash>

namespace Qtilities {
    namespace Core {  
        class Observer;

        /*!
        \struct ObserverDotWriterPrivateData
        \brief Structure used by ObserverDotWriter to store private data.
          */
        struct ObserverDotWriterPrivateData;

        /*!
          \class ObserverDotWriter
          \brief The ObserverDotWriter class generate dot scripts to create \p dot graphs for an observer hierarhcy.

          The ObserverDotWriter class is used to generate scripts in the \p dot language which can be parsed by
          a number of program to create \p dot graphs. For an overview of the \p dot language, a good place to start
          is <a href="http://www.graphviz.org/Documentation/dotguide.pdf">The Dot Guide</a>. To find software
          which can generate graphs from \p dot scripts, a good place to start is <a href="http://www.graphviz.org/">Graphviz</a>.

          \section observer_dot_graph_creating_a_graph Creating A Graph
          To create a \p dot script for an observer tree is easy. For example:
\code
// Create tree structure:
TreeNode* node = new TreeNode("Root Node");
node->addItem("Item 1");
node->addItem("Item 2");
node->addItem("Item 3");

// Create a dot script for this tree:
ObserverDotWriter dotGraph(node);
dotGraph.generateDotScript();
dotGraph.saveToFile("output_file.gv");
\endcode

        The resulting \p dot script looks like this:
\code
digraph "Root Node" {
    0 [label="Root Node"];
    0 -> 1;
    0 -> 2;
    0 -> 3;
    1 [label="Item 1"];
    2 [label="Item 2"];
    3 [label="Item 3"];
}
\endcode

        Next we can run the script using the \p Gvedit application found in the \p GraphViz distribution to create graph.
        Different layout engines are available for the scripts created by ObserverDotWriter and here we will only show a couple of them.
        First the output of the \p dot layout engine is shown:
        \image html observer_dot_graph_example_tree_dot.jpg "Example Graph (Dot Layout Engine)"

        Next the output of the \p circo layout engine is shown:
        \image html observer_dot_graph_example_tree_circo.jpg "Example Graph (Circo Layout Engine)"

        Since tree items can be attached to multiple nodes, it is possible to create more complex graphs easily as well.
        Lets look at an example:

\code
// Create tree structure:
TreeNode* node = new TreeNode("Root Node");
TreeNode* nodeA = node->addNode("Node A");
TreeNode* nodeB = node->addNode("Node B");
nodeA->addItem("Item 1");
nodeA->addItem("Item 2");
TreeItem* sharedItem = nodeA->addItem("Shared Item");
nodeB->attachSubject(sharedItem);
nodeB->addItem("Item 3");
nodeB->addItem("Item 4");

// Create a dot script for this tree:
ObserverDotWriter dotGraph(node);
dotGraph.generateDotScript();
dotGraph.saveToFile(QApplication::applicationDirPath() + "/output_file.gv");
\endcode

        The resulting \p dot script looks like this:

\code
digraph "Root Node" {
    0 [label="Root Node"];
    0 -> 1;
    0 -> 5;
    1 [label="Node A"];
    1 -> 2;
    1 -> 3;
    1 -> 4;
    2 [label="Item 1"];
    3 [label="Item 2"];
    4 [label="Shared Item"];
    5 [label="Node B"];
    5 -> 4;
    5 -> 6;
    5 -> 7;
    6 [label="Item 3"];
    7 [label="Item 4"];
}
\endcode

        The graph produced by the \p dot layout engine is shown below:

        \image html observer_dot_graph_example_complex_tree_dot.jpg "Example Graph (Dot Layout Engine)"

        The same graph produced by the \p twopi layout engine is shown below:

        \image html observer_dot_graph_example_complex_tree_twopi.jpg "Example Graph (Twopi Layout Engine)"

        \section observer_dot_graph_customizing Customizing A Graph

        The ObserverDotWriter class allows complete customization of the graph it creates. Before looking at the different
        ways to customize a graph, a short overview of the parts of the graph to which properties can be applied must be given.
        - The graph itself
        - Nodes in the graph
        - Connections between nodes (edge attributes)

        The properties that can be applied to each of these are detailed in the <a href="http://www.graphviz.org/Documentation/dotguide.pdf">The Dot Guide</a> and
        will not be repeated here. For all properties the defaults of the \p dot language is used since ObserverDotWriter does
        not add them to the script if they are not specified. The only aspect of the graph that is set explicitly is
        the type of graph: undirected or directed which can be specified using setGraphType().

        \subsection observer_dot_graph_attributes Graph Attributes

        It is possible to add any graph attribute specified in the dot language to a your graph. For example:

\code
// Create tree structure:
TreeNode* node = new TreeNode("Root Node");
node->addItem("Item 1");
node->addItem("Item 2");
node->addItem("Item 3");

// Create a dot script for this tree:
ObserverDotWriter dotGraph(node);
dotGraph.generateDotScript();
dotGraph.saveToFile("output_file.gv");
\endcode

        The resulting \p dot script looks like this:
\code
digraph "Root Node" {
    0 [label="Root Node" color="red"];
    0 -> 1;
    0 -> 2;
    0 -> 3;
    1 [label="Item 1"];
    2 [label="Item 2"];
    3 [label="Item 3"];
}
\endcode

        \subsection observer_dot_graph_node_attributes Node Attributes

        It is possible to add any node attribute specified in the dot language to a node in your graph. For example:

\code
// Create tree structure:
TreeNode* node = new TreeNode("Root Node");
node->addItem("Item 1");
node->addItem("Item 2");
node->addItem("Item 3");

// Create a dot script for this tree:
ObserverDotWriter dotGraph(node);
dotGraph.addNodeAttribute(node,"color","red");
dotGraph.generateDotScript();
dotGraph.saveToFile("output_file.gv");
\endcode

        The resulting \p dot script looks like this:
\code
digraph "Root Node" {
    0 [label="Root Node" color="red"];
    0 -> 1;
    0 -> 2;
    0 -> 3;
    1 [label="Item 1"];
    2 [label="Item 2"];
    3 [label="Item 3"];
}
\endcode


        <i>This class was added in %Qtilities v0.3.</i>
          */
        class QTILIITES_CORE_SHARED_EXPORT ObserverDotWriter : public QObject
        {
            Q_OBJECT
            Q_ENUMS(GraphType)

        public:
            //! The possible graph types.
            /*!
              Default is Directed.

              \sa setGraphType(), graphType()
              */
            enum GraphType {
                Directed =    0, /*!< Directed graph. */
                Undirected =  1  /*!< Undirected graph. */
            };

            //! Default constructor
            /*!
               \param observer The observer for which the script must be generated. The observer will also become the parent of this class.
               \param graph_type The type of graph.
              */
            ObserverDotWriter(Observer* observer = 0, GraphType graph_type = Directed);
            //! Copy constructor.
            ObserverDotWriter(const ObserverDotWriter& other);
            //! Overloaded = operator.
            void operator=(const ObserverDotWriter& other);
            //! Destructor.
            virtual ~ObserverDotWriter();

            //! Sets the observer context.
            bool setObserverContext(Observer* observer);
            //! Gets a pointer to the observer context.
            Observer* observerContext() const;

            //! Gets the graph type.
            GraphType type() const;
            //! Sets the graph type.
            void setType(GraphType type);

            //! Saves the dot script to a file.
            /*!
              Function which will write the dot script generated using the generateDotScript() function to the specified file.

              \note If no observer context have been specified, this function will return false.
              */
            virtual bool saveToFile(const QString& fileName) const;
            //! Function which will generate the dot script for the specified observer context.
            /*!
              \note If no observer context have been specified, this function will return QString().
              */
            QString generateDotScript() const;

            //! Adds a node attribute to a node in the graph.
            /*!
              See <a href="http://www.graphviz.org/Documentation/dotguide.pdf">The Dot Guide</a> for a list of possible node attributes. Overwrites existing attribute with the same name if it exists. If the node you specify is not part of the observer tree (see observerContext()) then this function will fail and return false.

              \param node A pointer to the node.
              \param attribute The name of the attribute to be added.
              \param value The value of the attribute.

              \returns True if successfull, false otherwise.
              */
            bool addNodeAttribute(QObject* node, const QString& attribute, const QString& value);
            //! Removes a node attribute from a node in the graph.
            /*!
              \param node A pointer to the node.
              \param attribute The name of the attribute to be removed.

              \returns True if successfull, false otherwise.
              */
            bool removeNodeAttribute(QObject* node, const QString& attribute);
            //! Returns a list of all attributes on a node in the graph.
            /*!
              \returns A QHash with attribute names a keys and their corresponding values as values.
              */
            QHash<QString,QString> nodeAttributes(QObject* node) const;

            //! Adds an edge attribute between two nodes in the graph.
            /*!
              See <a href="http://www.graphviz.org/Documentation/dotguide.pdf">The Dot Guide</a> for a list of possible edge attributes. Overwrites existing attribute with the same name if it exists. If either of the nodes you specify are not part of the top level observer tree (see observerContext()) then this function will fail and return false.

              \param parent A pointer to the parent observer.
              \param child The child underneath the parent observer.
              \param attribute The name of the attribute to be added.
              \param value The value of the attribute.

              \returns True if successfull, false otherwise.
              */
            bool addEdgeAttribute(Observer* parent, QObject* child, const QString& attribute, const QString& value);
            //! Removes a node attribute from a node in the graph.
            /*!
              \param node A pointer to the node.
              \param attribute The name of the attribute to be removed.

              \returns True if successfull, false otherwise.
              */
            bool removeEdgeAttribute(Observer* parent, QObject* child, const QString& attribute);
            //! Returns a list of all attributes on a node in the graph.
            /*!
              \returns A QHash with attribute names a keys and their corresponding values as values.
              */
            QHash<QString,QString> edgeAttributes(Observer* parent, QObject* child) const;

            //! Adds a node attribute to a node in the graph.
            /*!
              See <a href="http://www.graphviz.org/Documentation/dotguide.pdf">The Dot Guide</a> for a list of possible graph attributes. Overwrites existing attribute with the same name if it exists.

              \param attribute The name of the attribute to be added.
              \param value The value of the attribute.

              \returns True if successfull, false otherwise.
              */
            bool addGraphAttribute(const QString& attribute, const QString& value);
            //! Removes a node attribute from a node in the graph.
            /*!
              \param attribute The name of the attribute to be removed.

              \returns True if successfull, false otherwise.
              */
            bool removeGraphAttribute(const QString& attribute);
            //! Returns a list of all attributes on a node in the graph.
            /*!
              \returns A QHash with attribute names a keys and their corresponding values as values.
              */
            QHash<QString,QString> graphAttributes() const;

        private:
            ObserverDotWriterPrivateData* d;
        };

    }
}

#endif // OBSERVERDOTGRAPH_H