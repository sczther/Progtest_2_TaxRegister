#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <algorithm>
#endif /* __PROGTEST__ */

//structure for holding information about a single person
//always linked to by shared_ptr, so we don't need to manage memory
//accessed by two sorted vectors of pointers, by AccountNum and by <Name,Adress>
struct Person
{
  std::string     AccountNum,
                  Name,
                  Address;
  int             Income,
                  Expense;
  Person() :AccountNum(NULL), Name(NULL), Address(NULL), Income(0), Expense(0) {};
  Person(std::string nName, std::string nAdr, std::string nAcc) :
                  AccountNum(nAcc),
                  Name(nName),
                  Address(nAdr),
                  Income(0),
                  Expense(0) {};                  
};

//iterator, uses default std::vector iterator as we have a sorted one ready
class CIterator
{
  public:
    bool                          AtEnd                         ( void ) const;
    void                          Next                          ( void );
    std::string                   Name                          ( void ) const;
    std::string                   Addr                          ( void ) const;
    std::string                   Account                       ( void ) const;

    CIterator(std::vector<std::shared_ptr<Person>>::const_iterator nIt, std::vector<std::shared_ptr<Person>>::const_iterator nEnd) :
                                                                m_Iterator(nIt),
                                                                m_End(nEnd) {};
  private:
    std::vector<std::shared_ptr<Person>>::const_iterator m_Iterator;
    const std::vector<std::shared_ptr<Person>>::const_iterator m_End; //for checking if at end of vector
};

//implements working with a register of people, tracking two keys and two values
//the people are implemented as a Person struct, pointed to by two sorted vectors
//data is always in a single place in memory, shared_ptr deletes once unneeded
//methods return false if specified Person doesn't exist (or exists already when creating)
class CTaxRegister
{
  public:
    bool                     Birth                         ( const std::string    & name,
                                                             const std::string    & addr,
                                                             const std::string    & account );
    bool                     Death                         ( const std::string    & name,
                                                             const std::string    & addr );
    bool                     Income                        ( const std::string    & account,
                                                             int               amount );
    bool                     Income                        ( const std::string    & name,
                                                             const std::string    & addr,
                                                             int               amount );
    bool                     Expense                       ( const std::string    & account,
                                                             int               amount );
    bool                     Expense                       ( const std::string    & name,
                                                             const std::string    & addr,
                                                             int               amount );
    bool                     Audit                         ( const std::string    & name,
                                                             const std::string    & addr,
                                                             std::string          & account,
                                                             int             & sumIncome,
                                                             int             & sumExpense ) const;
    CIterator                ListByName                    ( void ) const;
  private:
    std::vector<std::shared_ptr<Person>> m_RegisterByName;
    std::vector<std::shared_ptr<Person>> m_RegisterByAccount;
};

//Compare used for <name,adress> pair and a Person
bool operator < (const std::shared_ptr<Person> & lhs, const std::pair<std::string, std::string> & rhs)
{
  std::pair<std::string, std::string> tmp(lhs->Name, lhs->Address);
  return tmp < rhs;
}

//Compare used for account number and Person
bool operator < (const std::shared_ptr<Person> & lhs, const std::string & rhs)
{
  return lhs->AccountNum < rhs;
}

//create new person and insert sorted
bool     CTaxRegister::Birth   ( const std::string & name, const std::string & addr, const std::string & account )
{
  std::pair<std::string,std::string> key(name, addr);
  auto ptr = std::shared_ptr<Person>(new Person(name, addr, account)); //allocate the Person
  auto posName=std::lower_bound(m_RegisterByName.begin(),m_RegisterByName.end(),key);
  auto posAcc =std::lower_bound(m_RegisterByAccount.begin(),m_RegisterByAccount.end(),account);
  
  //empty Register will always add
  if( m_RegisterByName.empty())
  {
    m_RegisterByName.push_back(ptr);
    m_RegisterByAccount.push_back(ptr);
    return true;
  }
  
  //check if <name,adress> exists already
  if(posName != m_RegisterByName.end() && (*posName)->Name == name && (*posName)->Address == addr)
  {
    return false;
  }
  
  //check if account exists already
  if(posAcc != m_RegisterByAccount.end() && (*posAcc)->AccountNum == account)
  {
    return false;
  }
  
  m_RegisterByName.insert(posName,ptr);
  m_RegisterByAccount.insert(posAcc,ptr);

  return true;
}

//remove person
bool     CTaxRegister::Death   ( const std::string & name, const std::string & addr )
{
  std::pair<std::string,std::string> key(name, addr); 
  auto pos=std::lower_bound(m_RegisterByName.begin(),m_RegisterByName.end(),key);
  
  //check if person exists
  if(pos == m_RegisterByName.end() || (*pos)->Name != name || (*pos)->Address != addr)
  {
    return false;
  }  
  
  //find in other Register
  auto posAcc=std::lower_bound(m_RegisterByAccount.begin(),m_RegisterByAccount.end(),(*pos)->AccountNum);

  m_RegisterByName.erase(pos);
  m_RegisterByAccount.erase(posAcc);

  return true;
}

//add to income of person
bool     CTaxRegister::Income  ( const std::string & account, int amount )
{
  auto pos=std::lower_bound(m_RegisterByAccount.begin(),m_RegisterByAccount.end(),account);
  
  //check if person exists
  if(pos == m_RegisterByAccount.end() || (*pos)->AccountNum != account)
  {
    return false;
  }

  (*pos)->Income += amount;

  return true;
}

//add to income of person
bool     CTaxRegister::Income  ( const std::string & name, const std::string & addr, int amount )
{
  std::pair<std::string,std::string> key(name, addr);
  auto pos = std::lower_bound(m_RegisterByName.begin(),m_RegisterByName.end(),key);
  
  //check if person exists
  if(pos == m_RegisterByName.end() || (*pos)->Address != addr || (*pos)->Name != name)
  {
    return false;
  }

  (*pos)->Income += amount;  

  return true;
}

//add to expense of person
bool     CTaxRegister::Expense  ( const std::string & account, int amount )
{
  auto pos=std::lower_bound(m_RegisterByAccount.begin(),m_RegisterByAccount.end(),account);
  
  //check if person exists
  if(pos == m_RegisterByAccount.end() || (*pos)->AccountNum != account)
  {
    return false;
  }

  (*pos)->Expense += amount;

  return true;
}

//add to expense of person
bool     CTaxRegister::Expense  ( const std::string & name, const std::string & addr, int amount )
{
  std::pair<std::string,std::string> key(name, addr);
  auto pos = std::lower_bound(m_RegisterByName.begin(),m_RegisterByName.end(),key);
  
  //check if person exists
  if(pos == m_RegisterByName.end() || (*pos)->Address != addr || (*pos)->Name != name)
  {
    return false;
  }

  (*pos)->Expense += amount;

  return true;
}

//get all data about person
bool     CTaxRegister::Audit     ( const std::string & name, const std::string & addr, std::string & account, int & sumIncome, int & sumExpense ) const
{
  std::pair<std::string,std::string> key(name, addr);
  auto pos = std::lower_bound(m_RegisterByName.begin(),m_RegisterByName.end(),key);
  
  //check if person exists
  if(pos == m_RegisterByName.end() || (*pos)->Address != addr || (*pos)->Name != name)
  {
    return false;
  }

  sumExpense = (*pos)->Expense;
  sumIncome  = (*pos)->Income;
  account    = (*pos)->AccountNum;

  return true;
}

//return iterator over sorted vector
CIterator CTaxRegister::ListByName  ( void ) const
{
  return CIterator(m_RegisterByName.begin(),m_RegisterByName.end());
}

//check if iterator is valid
bool          CIterator::AtEnd        ( void ) const
{
  if(m_Iterator == m_End)
  {
    return true;
  }
  return false;
}

//iterate to next position
void          CIterator::Next         ( void )
{
  m_Iterator++;
}

//get name from current position
std::string   CIterator::Name         ( void ) const
{
  return (*m_Iterator)->Name;
}

//get adress from current position
std::string   CIterator::Addr         ( void ) const
{
  return (*m_Iterator)->Address;
}

//get account number from current position
std::string   CIterator::Account      ( void ) const
{
  return (*m_Iterator)->AccountNum;
}


#ifndef __PROGTEST__
int main ( void )
{
  std::string acct;
  int    sumIncome, sumExpense;
  CTaxRegister b1;
  assert ( b1 . Birth ( "John Smith", "Oak Road 23", "123/456/789" ) );
  assert ( b1 . Birth ( "Jane Hacker", "Main Street 17", "Xuj5#94" ) );
  assert ( b1 . Birth ( "Peter Hacker", "Main Street 17", "634oddT" ) );
  assert ( b1 . Birth ( "John Smith", "Main Street 17", "Z343rwZ" ) );
  assert ( b1 . Income ( "Xuj5#94", 1000 ) );
  assert ( b1 . Income ( "634oddT", 2000 ) );
  assert ( b1 . Income ( "123/456/789", 3000 ) );
  assert ( b1 . Income ( "634oddT", 4000 ) );
  assert ( b1 . Income ( "Peter Hacker", "Main Street 17", 2000 ) );
  assert ( b1 . Expense ( "Jane Hacker", "Main Street 17", 2000 ) );
  assert ( b1 . Expense ( "John Smith", "Main Street 17", 500 ) );
  assert ( b1 . Expense ( "Jane Hacker", "Main Street 17", 1000 ) );
  assert ( b1 . Expense ( "Xuj5#94", 1300 ) );
  assert ( b1 . Audit ( "John Smith", "Oak Road 23", acct, sumIncome, sumExpense ) );
  assert ( acct == "123/456/789" );
  assert ( sumIncome == 3000 );
  assert ( sumExpense == 0 );
  assert ( b1 . Audit ( "Jane Hacker", "Main Street 17", acct, sumIncome, sumExpense ) );
  assert ( acct == "Xuj5#94" );
  assert ( sumIncome == 1000 );
  assert ( sumExpense == 4300 );
  assert ( b1 . Audit ( "Peter Hacker", "Main Street 17", acct, sumIncome, sumExpense ) );
  assert ( acct == "634oddT" );
  assert ( sumIncome == 8000 );
  assert ( sumExpense == 0 );
  assert ( b1 . Audit ( "John Smith", "Main Street 17", acct, sumIncome, sumExpense ) );
  assert ( acct == "Z343rwZ" );
  assert ( sumIncome == 0 );
  assert ( sumExpense == 500 );


  CIterator it = b1 . ListByName ();
  assert ( ! it . AtEnd ()
           && it . Name () == "Jane Hacker"
           && it . Addr () == "Main Street 17"
           && it . Account () == "Xuj5#94" );
  it . Next ();
  assert ( ! it . AtEnd ()
           && it . Name () == "John Smith"
           && it . Addr () == "Main Street 17"
           && it . Account () == "Z343rwZ" );
  it . Next ();
  assert ( ! it . AtEnd ()
           && it . Name () == "John Smith"
           && it . Addr () == "Oak Road 23"
           && it . Account () == "123/456/789" );
  it . Next ();
  assert ( ! it . AtEnd ()
           && it . Name () == "Peter Hacker"
           && it . Addr () == "Main Street 17"
           && it . Account () == "634oddT" );
  it . Next ();
  assert ( it . AtEnd () );

  assert ( b1 . Death ( "John Smith", "Main Street 17" ) );

  CTaxRegister b2;
  assert ( b2 . Birth ( "John Smith", "Oak Road 23", "123/456/789" ) );
  assert ( b2 . Birth ( "Jane Hacker", "Main Street 17", "Xuj5#94" ) );
  assert ( !b2 . Income ( "634oddT", 4000 ) );
  assert ( !b2 . Expense ( "John Smith", "Main Street 18", 500 ) );
  assert ( !b2 . Audit ( "John Nowak", "Second Street 23", acct, sumIncome, sumExpense ) );
  assert ( !b2 . Death ( "Peter Nowak", "5-th Avenue" ) );
  assert ( !b2 . Birth ( "Jane Hacker", "Main Street 17", "4et689A" ) );
  assert ( !b2 . Birth ( "Joe Hacker", "Elm Street 23", "Xuj5#94" ) );
  assert ( b2 . Death ( "Jane Hacker", "Main Street 17" ) );
  assert ( b2 . Birth ( "Joe Hacker", "Elm Street 23", "Xuj5#94" ) );
  assert ( b2 . Audit ( "Joe Hacker", "Elm Street 23", acct, sumIncome, sumExpense ) );
  assert ( acct == "Xuj5#94" );
  assert ( sumIncome == 0 );
  assert ( sumExpense == 0 );
  assert ( !b2 . Birth ( "Joe Hacker", "Elm Street 23", "AAj5#94" ) );

  return 0;
}
#endif /* __PROGTEST__ */
