/*======================================================================
 *
 *  C++ Factory Framework Example
 *
 *  Author: Edward Patel, Sweden
 *  Copyright (c) 2001, All rights reserved
 *
 *  TODO: Error managment
 *
 *======================================================================*/

#include <iostream>

using namespace std;

/*======================================================================*/
// Some short forward declarations

class ObjectRoot;
typedef ObjectRoot *ObjectRootPtr;

/*======================================================================*/
// Simple collection class

class ObjectCollection {
public:
  ObjectCollection();
  virtual ~ObjectCollection();

  int add(ObjectRoot *obj);
  ObjectRoot *find(int id);
  int find(ObjectRoot *obj);
      
protected:
  struct ObjectNode {
    ObjectNode *next;
    int id;
    ObjectRoot *obj;
  };
  ObjectNode *head;
  int nextId;
};

ObjectCollection::ObjectCollection() : head(NULL), nextId(1) {
}

ObjectCollection::~ObjectCollection() {
  while (head) {
    ObjectNode *tmp(head);
    head = head->next;
    delete tmp;
  }
}

int ObjectCollection::add(ObjectRoot *obj) {
  ObjectNode *tmp(new ObjectNode);
  if (tmp) {
    tmp->next = head;
    tmp->id = nextId++;
    tmp->obj = obj;
    head = tmp;
    return tmp->id;
  }
  return 0;
}

ObjectRoot *ObjectCollection::find(int id) {
  ObjectNode *tmp(head);
  while (tmp) {
    if (tmp->id == id)
      return tmp->obj;
    tmp = tmp->next;
  }
  return NULL;
}

int ObjectCollection::find(ObjectRoot *obj) {
  ObjectNode *tmp(head);
  while (tmp) {
    if (tmp->obj == obj)
      return tmp->id;
    tmp = tmp->next;
  }
  return 0;
}

/*======================================================================*/

class Factory {
public:
  Factory();
  virtual ~Factory();

  static Factory    *getFactory();
  static int         registerCreator(const char *name, ObjectRoot *(*cf)());
  static ObjectRoot *createObject(char *name);
  static void        resetObjectCollection();
  static ObjectRoot *readObject(istream &is);
  static void        writeObject(ostream &os, ObjectRoot *obj);

protected:
  struct CreatorEntry {
    CreatorEntry *next;
    const char *name;
    ObjectRoot *(*cf)();
  };
  CreatorEntry *creatorList;
  static Factory *theFactory;
  ObjectCollection *objects;
};

#define METADECL(_type)             \
  protected:                        \
  virtual const char *getType()

#define METAIMPL(_type)                             \
  const char *_type::getType() { return # _type; }  \
  REGISTER(_type)

class ObjectRoot {

  METADECL(ObjectRoot);

public:
  ObjectRoot();
  virtual ~ObjectRoot();

  void read(istream &is);
  virtual void doRead(istream &is);
  void write(ostream &os);
  virtual void doWrite(ostream &os);

  virtual void doWhenRead() {}

  friend ostream &operator<<(ostream &os, ObjectRoot &obj);
  friend ostream &operator<<(ostream &os, ObjectRoot *obj);
  friend istream &operator>>(istream &is, ObjectRoot &obj);
  friend istream &operator>>(istream &is, ObjectRootPtr &obj);
};

/*======================================================================*/

Factory *Factory::theFactory = NULL;

Factory::Factory() : creatorList(NULL), objects(new ObjectCollection) {
}

Factory::~Factory() {
  while (creatorList) {
    CreatorEntry *tmp(creatorList);
    creatorList = creatorList->next;
    delete tmp;
  }
  if (objects)
    delete objects;
}

Factory *Factory::getFactory() {
  if (!theFactory)
    theFactory = new Factory;
  return theFactory;
}

int Factory::registerCreator(const char *name, ObjectRoot *(*cf)()) {
  Factory *f(getFactory());
  CreatorEntry *tmp(new CreatorEntry);
  if (tmp) {
    tmp->next = f->creatorList;
    tmp->name = name;
    tmp->cf = cf;
    f->creatorList = tmp;
  }
}

ObjectRoot *Factory::createObject(char *name) {
  Factory *f(getFactory());
  CreatorEntry *tmp(f->creatorList);   
  while (tmp) {
    if (!strcmp(tmp->name, name))
      return tmp->cf();
    tmp = tmp->next;
  }
  return NULL;
}

void Factory::resetObjectCollection() {
  Factory *f(getFactory());
  if (f->objects)
    delete f->objects;
  f->objects = new ObjectCollection;
}

static char readBuffer[256];

ObjectRoot *Factory::readObject(istream &is) {
  Factory *f(getFactory());
  is >> readBuffer;
  if (f->objects && !strcmp("#", readBuffer)) {
    int id;
    is >> id;
    return f->objects->find(id);
  } else {
    ObjectRoot *tmp(f->createObject(readBuffer));
    if (tmp) {
      if (f->objects)
    f->objects->add(tmp);
      tmp->read(is);
      tmp->doWhenRead();
    }
    return tmp;
  }
  return NULL;
}

void Factory::writeObject(ostream &os, ObjectRoot *obj) {
  Factory *f(getFactory());
  if (f->objects) {
    int id(f->objects->find(obj));
    if (id)
      os << "# " << id << endl;
    else {
      f->objects->add(obj);
      obj->write(os);
    }
  }
}

#define REGISTER(_type)                                                 \
  static ObjectRoot *_type ## FactoryCreator() { return new _type; };   \
  static int _type ## CreatorAutoRegHook = Factory::registerCreator(#_type, _type ## FactoryCreator)

/*======================================================================*/

METAIMPL(ObjectRoot);

ObjectRoot::ObjectRoot() {
}

ObjectRoot::~ObjectRoot() {
}

void ObjectRoot::read(istream &is) {
  is >> readBuffer;
  if (!strcmp("{", readBuffer)) {
    doRead(is);
  }
  is >> readBuffer;
  // should be a "}"
}

void ObjectRoot::doRead(istream &is) {
  // empty
}

void ObjectRoot::write(ostream &os) {
  os << getType() << " {" << endl;
  doWrite(os);
  os << "}" << endl;
}

void ObjectRoot::doWrite(ostream &os) {
  // empty
}

/*======================================================================*/
// Stream operators for ObjectRoot classes

ostream &operator<<(ostream &os, ObjectRoot &obj) {
  obj.write(os);
  return os;
}

ostream &operator<<(ostream &os, ObjectRoot *obj) {
  if (obj)
    Factory::getFactory()->writeObject(os, obj);
  else
    os << "(null)" << endl;
  return os;
}

istream &operator>>(istream &is, ObjectRoot &obj) {
  is >> readBuffer; // get type identifier
  obj.read(is);
  return is;
}

istream &operator>>(istream &is, ObjectRootPtr &obj) {
  ObjectRoot *tmp(Factory::getFactory()->readObject(is));
  obj = tmp;
  return is;
}

/*======================================================================

  #    #   ####   ######  #####
  #    #  #       #       #    #
  #    #   ####   #####   #    #
  #    #       #  #       #####
  #    #  #    #  #       #   #
   ####    ####   ######  #    #

  ======================================================================*/
// User added classes, to be read and written to file

class MyClassOne : public ObjectRoot { // is-a ObjectRoot ofcourse

  // This will add a method for getting the name of the type
  METADECL(MyClassOne);

public:
  MyClassOne();
  ~MyClassOne();

  void doWhenRead() { cout << "I'm alive!!!" << endl; }

};

// This will auto register a creator function to the Factory
METAIMPL(MyClassOne);

MyClassOne::MyClassOne() {
}

MyClassOne::~MyClassOne() {
}

class MyClassTwo : public ObjectRoot {
      
  METADECL(MyClassTwo);

public:
  MyClassTwo();
  ~MyClassTwo();

  ObjectRoot *anObject; // has-a 
  int aValue;
      
  void doRead(istream &is);
  void doWrite(ostream &os);
};

METAIMPL(MyClassTwo);

MyClassTwo::MyClassTwo() : anObject(NULL), aValue(0) {
}

MyClassTwo::~MyClassTwo() {
}

void MyClassTwo::doRead(istream &is) {
  // should also do a <SuperClass>::doRead(is), is empty here
  ObjectRoot::doRead(is);
  is >> anObject;
  is >> aValue;
}

void MyClassTwo::doWrite(ostream &os) {
  // should also do a <SuperClass>::doWrite(os), is empty here
  ObjectRoot::doWrite(os);
  os << anObject;
  os << aValue << endl;
}

/*======================================================================

  #    #    ##       #    #    #
  ##  ##   #  #      #    ##   #
  # ## #  #    #     #    # #  #
  #    #  ######     #    #  # #
  #    #  #    #     #    #   ##
  #    #  #    #     #    #    #

  ======================================================================*/

#include <fstream> // for ifstream and ofstream

int main(int argc, char *argv[]) {
  ifstream ifile("factory.dat");
  ObjectRoot *obj;

  cout << "Reading data:" << endl;
  ifile >> obj;

  cout << "Writing read data:" << endl;
  Factory::resetObjectCollection();
  cout << obj;

  cout << "Creating data:" << endl;

  MyClassTwo *mct1 = new MyClassTwo;
  MyClassTwo *mct2 = new MyClassTwo;
  MyClassTwo *mct3 = new MyClassTwo;

  mct1->anObject = mct2;
  mct1->aValue = 1;

  mct2->anObject = mct3;
  mct2->aValue = 2;

  mct3->anObject = mct1; // this will create a circle, which this example can handle
  mct3->aValue = 3;

  cout << "Writing created data:" << endl;
  Factory::resetObjectCollection();
  cout << mct1;

  return 0;
}
