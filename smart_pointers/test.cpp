#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tfile.h"
#include "to_lower_case.h"

TEST(to_lower_case_test, perform )
{
  // Arrange
  to_lower_case & to_lower = to_lower_case::init() ;
  std::string str = "ТесТ тЕСт АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ!" ; 

  // Act
  str = to_lower( str );
  std::cout << "str: "<< str << "\n";
  // Assert
  EXPECT_STREQ( str.c_str(), "тест тест абвгдежзийклмнопрстуфхцчшщъыьэюя!") ; 
}

struct tfile_test : public testing::Test 
{
  // Arrange
  std::string str_root_dir ; 
  ptfile root_dir ;
  ttree tree ;

  void SetUp()  
  { 
    str_root_dir = "c:/root_dir" ;

    root_dir = tree.add_tree( str_root_dir ) ;
    tree.add_file( "file1" , root_dir ) ;
    tree.add_file( "file2" , root_dir ) ;
    ptfile dir2 = tree.add_dir( "dir2" , root_dir ) ;
    tree.add_file( "file21" , dir2 ) ;
    tree.add_file( "file22" , dir2 ) ;

  } 
  void TearDown() {  } // 
};

TEST_F(tfile_test, Test_tree)
{
  // Act
    std::string s = "" ;
    traverse_tree( root_dir , [ &s ] (ptfile & pfile ) 
      { s += pfile->get_name() ; } ) ;

  // Assert
    ASSERT_STREQ( s.c_str() , "file1file2dir2file21file22"); 
}

TEST_F(tfile_test, Test_getfullname )
{
  // Act
    std::string s = "" ;
    traverse_tree( root_dir , [ &s ] (ptfile & pfile ) 
      { s += pfile->get_full_name() ; } ) ;

  // Assert
  ASSERT_STREQ( s.c_str() , "c:/root_dir/file1c:/root_dir/file2c:/root_dir/dir2/c:/root_dir/dir2/file21c:/root_dir/dir2/file22") ; 
}

TEST_F( tfile_test , Test_find_str )
{
  // Act
  std::string s = "" ;
  std::vector< ptfile > res = find_str( tree , "file21" ) ;

  for ( ptfile & pfile : res )
  {
    s+= pfile.get()->get_full_name() ;
  }

  // Assert
  ASSERT_STREQ( s.c_str() , "c:/root_dir/dir2/file21") ; 
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  

  return RUN_ALL_TESTS();
}