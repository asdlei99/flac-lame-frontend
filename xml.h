/**
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * Copyright 2017-present Rodrigo Cesar de Freitas Dias
 * This library is released under the MIT License
 */

#pragma once
#include <string>
#include "com.h"
#include "held_map.h"
#include <MsXml2.h>
#pragma comment(lib, "msxml2.lib")

namespace wl {

// XML wrapper class to MSXML2 Windows library.
class xml final {
public:
	class node final {
	public:
		std::wstring name;
		std::wstring value;
		held_map<std::wstring, std::wstring> attrs;
		std::vector<node> children;

		void clear() noexcept {
			this->name.clear();
			this->value.clear();
			this->attrs.clear();
			this->children.clear();
		}

		std::vector<node*> children_by_name(const wchar_t* elemName) noexcept {
			std::vector<node*> nodeBuf;
			for (node& node : this->children) {
				if (!lstrcmpiW(node.name.c_str(), elemName)) { // case-insensitive match
					nodeBuf.emplace_back(&node);
				}
			}
			return nodeBuf;
		}

		std::vector<node*> children_by_name(const std::wstring& elemName) noexcept {
			return this->children_by_name(elemName.c_str());
		}

		node* first_child_by_name(const wchar_t* elemName) noexcept {
			for (node& node : this->children) {
				if (!lstrcmpiW(node.name.c_str(), elemName)) { // case-insensitive match
					return &node;
				}
			}
			return nullptr; // not found
		}

		node* first_child_by_name(const std::wstring& elemName) noexcept {
			return this->first_child_by_name(elemName.c_str());
		}
	};

private:
	com::lib _comLib{com::lib::init::LATER};

public:
	node root;

	xml() = default;
	xml(xml&& other) noexcept    { this->operator=(std::move(other)); }
	xml(const wchar_t* str)      { this->parse(str); }
	xml(const std::wstring& str) : xml(str.c_str()) { }

	xml& operator=(xml&& other) noexcept {
		this->root.clear();
		this->root = std::move(other.root);
		return *this;
	}

	xml& parse(const wchar_t* str) {
		this->_comLib.initialize();
		this->root.clear();

		// Create COM object for XML document.
		com::ptr<IXMLDOMDocument2> doc;
		doc.co_create_instance(CLSID_DOMDocument30, IID_IXMLDOMDocument);
		doc->put_async(FALSE);

		// Parse the XML string.
		VARIANT_BOOL vb = FALSE;
		HRESULT hr = doc->loadXML(static_cast<BSTR>(const_cast<wchar_t*>(str)), &vb);
		if (FAILED(hr)) {
			throw std::system_error(hr, std::system_category(),
				"IXMLDOMDocument::loadXML failed");
		}

		// Get document element and root node from XML.
		com::ptr<IXMLDOMElement> docElem;
		hr = doc->get_documentElement(docElem.pptr());
		if (FAILED(hr)) {
			throw std::system_error(hr, std::system_category(),
				"IXMLDOMDocument::get_documentElement failed");
		}

		com::ptr<IXMLDOMNode> rootNode;
		docElem.query_interface(IID_IXMLDOMNode, rootNode);
		this->root = _build_node(rootNode); // recursive, the whole tree is loaded into memory
		return *this;
	}

	xml& parse(const std::wstring& str) {
		return this->parse(str.c_str());
	}

private:
	static xml::node _build_node(com::ptr<IXMLDOMNode>& xmlDomNode) noexcept {
		xml::node ret;

		// Get node name.
		com::bstr bstrName;
		xmlDomNode->get_nodeName(bstrName.ptr());
		ret.name = bstrName.c_str();

		// Parse attributes of node, if any.
		ret.attrs = _read_attrs(xmlDomNode);

		// Process children, if any.
		VARIANT_BOOL vb = FALSE;
		xmlDomNode->hasChildNodes(&vb);
		if (vb) {
			com::ptr<IXMLDOMNodeList> nodeList;
			xmlDomNode->get_childNodes(nodeList.pptr());
			ret.children.reserve(_count_child_nodes(nodeList));

			int childCount = 0;
			long totalCount = 0;
			nodeList->get_length(&totalCount);

			for (long i = 0; i < totalCount; ++i) {
				com::ptr<IXMLDOMNode> child;
				nodeList->get_item(i, child.pptr());

				// Node can be text or an actual child node.
				DOMNodeType type = NODE_INVALID;
				child->get_nodeType(&type);
				if (type == NODE_TEXT) {
					com::bstr bstrText;
					xmlDomNode->get_text(bstrText.ptr());
					ret.value.append(bstrText.c_str());
				} else if (type == NODE_ELEMENT) {
					ret.children.emplace_back(_build_node(child)); // recursively
				} else {
					// (L"Unhandled node type: %d.\n", type);
				}
			}
		} else {
			// Assumes that only a leaf node can have text.
			com::bstr bstrText;
			xmlDomNode->get_text(bstrText.ptr());
			ret.value = bstrText.c_str();
		}
		return ret;
	}

	static held_map<std::wstring, std::wstring> _read_attrs(com::ptr<IXMLDOMNode>& xmlnode) noexcept {
		// Read attribute collection.
		com::ptr<IXMLDOMNamedNodeMap> attrs;
		xmlnode->get_attributes(attrs.pptr());

		long attrCount = 0;
		attrs->get_length(&attrCount);

		held_map<std::wstring, std::wstring> ret;
		ret.reserve(attrCount);

		for (long i = 0; i < attrCount; ++i) {
			com::ptr<IXMLDOMNode> attr;
			attrs->get_item(i, attr.pptr());

			DOMNodeType type = NODE_INVALID;
			attr->get_nodeType(&type);
			if (type == NODE_ATTRIBUTE) {
				com::bstr bstrName;
				attr->get_nodeName(bstrName.ptr()); // get attribute name				
				com::variant variNodeVal;
				attr->get_nodeValue(variNodeVal.ptr()); // get attribute value
				ret[bstrName.c_str()] = variNodeVal.get_str(); // add hash entry
			}
		}
		return ret;
	}

	static int _count_child_nodes(com::ptr<IXMLDOMNodeList>& nodeList) noexcept {
		int childCount = 0;
		long totalCount = 0;
		nodeList->get_length(&totalCount); // includes text and actual element nodes

		for (long i = 0; i < totalCount; ++i) {
			com::ptr<IXMLDOMNode> child;
			nodeList->get_item(i, child.pptr());

			DOMNodeType type = NODE_INVALID;
			child->get_nodeType(&type);
			if (type == NODE_ELEMENT) ++childCount;
		}
		return childCount;
	}
};

}//namespace wl
