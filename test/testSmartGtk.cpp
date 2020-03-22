#include <gtest/gtest.h>

#include <SmartGtk.hpp>

TEST(SmartGtk, CtorAndDtorAreCalled)
{
    bool ctorCalled = false, dtorCalled = false;
    auto createdObj = SmartGtk::MakeObject(
        [&ctorCalled](){ctorCalled = true; return new double();}, 
        [&dtorCalled](double* managedObject){dtorCalled = true; delete managedObject;});

    createdObj.reset();

    EXPECT_TRUE(ctorCalled);
    EXPECT_TRUE(dtorCalled);
}

static double* GetNullPtrDouble() {return NULL;}

/*! \brief sometimes creating a Gtk object fails, and you don't want to call 'g_object_unref' with a null pointer*/
TEST(SmartGtk, DtorNotCalledIfNULL)
{
    bool dtorCalled = false;
    auto createdObj = SmartGtk::MakeObject(
        GetNullPtrDouble,
        [&dtorCalled](double* managedObject){dtorCalled = true; delete managedObject;});

    createdObj.reset();

    EXPECT_FALSE(dtorCalled);
}