    Info<< "Reading field p_rgh\n" << endl;
    volScalarField p_rgh
    (
        IOobject
        (
            "p_rgh",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    Info<< "Reading field alpha\n" << endl;
    volScalarField alpha
    (
        IOobject
        (
            "alpha",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    Info<< "Reading field U\n" << endl;
    volVectorField U
    (
        IOobject
        (
            "U",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );


    Info<< "Reading transportProperties\n" << endl;

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );


    dimensionedScalar rhoc
    (
        transportProperties.lookup("rhoc")
    );

    dimensionedScalar rhod
    (
        transportProperties.lookup("rhod")
    );

    dimensionedScalar muc
    (
        transportProperties.lookup("muc")
    );

    dimensionedScalar plasticViscosityCoeff
    (
        transportProperties.lookup("plasticViscosityCoeff")
    );

    dimensionedScalar plasticViscosityExponent
    (
        transportProperties.lookup("plasticViscosityExponent")
    );

    dimensionedScalar yieldStressCoeff
    (
        transportProperties.lookup("yieldStressCoeff")
    );

    dimensionedScalar yieldStressExponent
    (
        transportProperties.lookup("yieldStressExponent")
    );

    dimensionedScalar yieldStressOffset
    (
        transportProperties.lookup("yieldStressOffset")
    );

    Switch BinghamPlastic
    (
        transportProperties.lookup("BinghamPlastic")
    );

    volScalarField rho
    (
        IOobject
        (
            "rho",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        (scalar(1) - alpha)*rhoc + alpha*rhod
    );

    volScalarField Alpha
    (
        IOobject
        (
            "Alpha",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        alpha*rhod/rho,
        alpha.boundaryField().types()
    );

    #include "compressibleCreatePhi.H"


    Info<< "Calculating field mul\n" << endl;
    volScalarField mul
    (
        IOobject
        (
            "mul",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        muc +
        plasticViscosity
        (
            plasticViscosityCoeff,
            plasticViscosityExponent,
            Alpha
        )
    );


    Info<< "Initialising field Vdj\n" << endl;
    volVectorField Vdj
    (
        IOobject
        (
            "Vdj",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedVector("0.0", U.dimensions(), vector::zero),
        U.boundaryField().types()
    );


    Info<< "Selecting Drift-Flux model " << endl;

    word VdjModel
    (
        transportProperties.lookup("VdjModel")
    );

    Info<< tab << VdjModel << " selected\n" << endl;

    const dictionary& VdjModelCoeffs
    (
        transportProperties.subDict(VdjModel + "Coeffs")
    );

    dimensionedVector V0
    (
        VdjModelCoeffs.lookup("V0")
    );

    dimensionedScalar a
    (
        VdjModelCoeffs.lookup("a")
    );

    dimensionedScalar a1
    (
        VdjModelCoeffs.lookup("a1")
    );

    dimensionedScalar alphaMin
    (
        VdjModelCoeffs.lookup("alphaMin")
    );



    IOdictionary RASProperties
    (
        IOobject
        (
            "RASProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );


    Switch turbulence
    (
        RASProperties.lookup("turbulence")
    );

    dictionary kEpsilonDict
    (
        RASProperties.subDictPtr("kEpsilonCoeffs")
    );

    dimensionedScalar Cmu
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "Cmu",
            kEpsilonDict,
            0.09
        )
    );

    dimensionedScalar C1
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "C1",
            kEpsilonDict,
            1.44
        )
    );

    dimensionedScalar C2
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "C2",
            kEpsilonDict,
            1.92
        )
    );

    dimensionedScalar C3
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "C3",
            kEpsilonDict,
            0.85
        )
    );

    dimensionedScalar alphak
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "alphaEps",
            kEpsilonDict,
            1.0
        )
    );

    dimensionedScalar alphaEps
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "alphaEps",
            kEpsilonDict,
            0.76923
        )
    );

    dictionary wallFunctionDict
    (
        RASProperties.subDictPtr("wallFunctionCoeffs")
    );

    dimensionedScalar kappa
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "kappa",
            wallFunctionDict,
            0.41
        )
    );

    dimensionedScalar E
    (
        dimensionedScalar::lookupOrAddToDict
        (
            "E",
            wallFunctionDict,
            9.0
        )
    );

    if (RASProperties.lookupOrDefault<Switch>("printCoeffs", false))
    {
        Info<< "kEpsilonCoeffs" << kEpsilonDict << nl
            << "wallFunctionCoeffs" << wallFunctionDict << endl;
    }


    nearWallDist y(mesh);

    Info<< "Reading field k\n" << endl;
    volScalarField k
    (
        IOobject
        (
            "k",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    Info<< "Reading field epsilon\n" << endl;
    volScalarField epsilon
    (
        IOobject
        (
            "epsilon",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    Info<< "Calculating field mut\n" << endl;
    volScalarField mut
    (
        IOobject
        (
            "mut",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        Cmu*rho*sqr(k)/epsilon
    );


    Info<< "Calculating field mu\n" << endl;
    volScalarField mu
    (
        IOobject
        (
            "mu",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mut + mul
    );


    Info<< "Calculating field (g.h)f\n" << endl;
    volScalarField gh("gh", g & mesh.C());
    surfaceScalarField ghf = surfaceScalarField("gh", g & mesh.Cf());

    volScalarField p
    (
        IOobject
        (
            "p",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        p_rgh + rho*gh
    );

    label pRefCell = 0;
    scalar pRefValue = 0.0;
    setRefCell
    (
        p,
        p_rgh,
        mesh.solutionDict().subDict("PISO"),
        pRefCell,
        pRefValue
    );

    if (p_rgh.needReference())
    {
        p += dimensionedScalar
        (
            "p",
            p.dimensions(),
            pRefValue - getRefCellValue(p, pRefCell)
        );
        p_rgh = p - rho*gh;
    }
